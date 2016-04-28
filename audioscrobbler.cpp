#include "mpdas.h"

#define APIKEY		"a0ed2629d3d28606f67d7214c916788d"
#define	SECRET		"295f31c5d28215215b1503fb0327cc01"
#define CURL_MAX_RETRIES 3
#define CURL_RETRY_DELAY 3 // Seconds

CAudioScrobbler* AudioScrobbler = 0;

#define CLEANUP()	_response.clear()

size_t writecb(void* ptr, size_t size, size_t nmemb, void *stream)
{
    AudioScrobbler->ReportResponse((char*)ptr, size*nmemb);
    return size*nmemb;
}

CAudioScrobbler::CAudioScrobbler()
{
    _failcount = 0;
    _authed = false;
    _response = "";
    _handle = curl_easy_init();
    if(!_handle) {
	eprintf("%s", "Could not initialize CURL.");
	exit(EXIT_FAILURE);
    }
}

CAudioScrobbler::~CAudioScrobbler()
{
    curl_easy_cleanup(_handle);
    curl_global_cleanup();
}

std::string CAudioScrobbler::GetServiceURL()
{
    if(Config->getService() == LibreFm) {
	return "https://libre.fm/2.0/";
    }
    return "http://ws.audioscrobbler.com/2.0/";
}

void CAudioScrobbler::OpenURL(std::string url, const char* postfields = 0, char* errbuf = 0)
{
    curl_easy_setopt(_handle, CURLOPT_DNS_CACHE_TIMEOUT, 0);
    curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, writecb);
    curl_easy_setopt(_handle, CURLOPT_TIMEOUT, 10);

    if(postfields) {
	curl_easy_setopt(_handle, CURLOPT_POST, 1);
	curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, postfields);
    }
    else
	curl_easy_setopt(_handle, CURLOPT_POST, 0);
    if(errbuf)
	curl_easy_setopt(_handle, CURLOPT_ERRORBUFFER, errbuf);

    curl_easy_setopt(_handle, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(_handle);

    // Sometimes last.fm likes to just timeout for no reason, leaving us hanging.
    // If this happens, retry a few times with a small delay.
    if (res != CURLE_OK) {
        eprintf("libcurl: (%d)", res);
        eprintf("%s", curl_easy_strerror(res));
        eprintf("Will retry %d times with a %d second delay.", CURL_MAX_RETRIES, CURL_RETRY_DELAY);

        int retries = 0;
        do {
            sleep(CURL_RETRY_DELAY);
            retries++;
            eprintf("Retry %d/%d", retries, CURL_MAX_RETRIES);
            res = curl_easy_perform(_handle);
        } while (res != CURLE_OK || retries < CURL_MAX_RETRIES);

        eprintf("Failed after %d retries, try again later.", CURL_MAX_RETRIES);
    }
}


void CAudioScrobbler::ReportResponse(char* buf, size_t size)
{
    _response.append(buf);
}

std::string CAudioScrobbler::CreateScrobbleMessage(int index, const CacheEntry& entry)
{
    const Song& song = entry.getSong();
    std::ostringstream msg, sigmsg ;
    std::string artist, title, album, array = "=";

    char* temp = 0;
    temp = curl_easy_escape(_handle, song.getArtist().c_str(), song.getArtist().length());
    artist = temp;
    curl_free(temp);
    temp = curl_easy_escape(_handle, song.getTitle().c_str(), song.getTitle().length());
    title = temp;
    curl_free(temp);
    temp = curl_easy_escape(_handle, song.getAlbum().c_str(), song.getAlbum().length());
    album = temp;
    curl_free(temp);

    msg << "&album" << array << album;
    msg << "&api_key=" << APIKEY;
    msg << "&artist" << array << artist;
    msg << "&duration" << array << song.getDuration();
    msg << "&method=track.Scrobble";
    msg << "&timestamp" << array << entry.getStartTime();
    msg << "&track" << array << title;
    msg << "&sk=" << _sessionid;

    array = "";

    sigmsg << "album" << array << song.getAlbum();
    sigmsg << "api_key" << APIKEY;
    sigmsg << "artist" << array << song.getArtist();
    sigmsg << "duration" << array << song.getDuration();
    sigmsg << "methodtrack.Scrobble";
    sigmsg << "sk" << _sessionid;
    sigmsg << "timestamp" << array << entry.getStartTime();
    sigmsg << "track" << array << song.getTitle();
    sigmsg << SECRET;

    std::string sighash(md5sum((char*)"%s", sigmsg.str().c_str()));
    msg << "&api_sig=" << sighash;

    return msg.str();
}

void CAudioScrobbler::Failure()
{
    _failcount += 1;
    if(_failcount >= 3) {
	eprintf("%s", "Re-Handshaking!");
	_failcount = 0;
	Handshake();
    }
}

bool CAudioScrobbler::CheckFailure(std::string response)
{
    bool retval = false;

    size_t start, end;
    start = _response.find("<error code=\"")+13;
    end = _response.find(">", start)-1;
    std::string errorcode = _response.substr(start, end-start);
    int code = strtol(errorcode.c_str(), 0, 10);

    eprintf("%s%i", "Code: ", code);

    switch(code) {
	case 3:
	    eprintf("Invalid Method. This should not happen.");
	    retval = true;
	    break;
	case 4:
	    eprintf("Authentication failed. Please check your login data.");
	    exit(EXIT_FAILURE);
	case 9:
	    eprintf("Invalid session key. Re-authenticating.");
	    retval = true;
	    _failcount = 3;
	    break;
	case 10:
	    eprintf("Invalid API-Key. Let's bugger off.");
	    exit(EXIT_FAILURE);
	case 16:
	    eprintf("The service is temporarily unavailable, we will try again later..");
	    retval = true;
	    break;
	case 26:
	    eprintf("Uh oh. Suspended API key - Access for your account has been suspended, please contact Last.fm");
	    exit(EXIT_FAILURE);
    }

    return retval;
}

bool CAudioScrobbler::Scrobble(const CacheEntry& entry)
{
    bool retval = false;
    if(!_authed) {
	eprintf("Handshake hasn't been done yet.");
	Handshake();
	return retval;
    }
    iprintf("Scrobbling: %s - %s", entry.getSong().getArtist().c_str(), entry.getSong().getTitle().c_str());

    OpenURL(GetServiceURL(), CreateScrobbleMessage(0, entry).c_str());
    if(_response.find("<lfm status=\"ok\">") != std::string::npos) {
	iprintf("%s", "Scrobbled successfully.");
	retval = true;
    }
    else if(_response.find("<lfm status=\"failed\">") != std::string::npos) {
	eprintf("%s%s", "Last.fm returned an error while scrobbling:\n", _response.c_str());
	if(CheckFailure(_response))
	    Failure();
    }
    CLEANUP();

    return retval;
}

bool CAudioScrobbler::LoveTrack(const Song& song, bool unlove)
{
    bool retval = false;

    char* artist = curl_easy_escape(_handle, song.getArtist().c_str(), 0);
    char* title = curl_easy_escape(_handle, song.getTitle().c_str(), 0);

    std::ostringstream query, sig;
    query << (unlove ? "method=track.unlove&" : "method=track.love&")
	<< "&track=" << title
	<< "&artist=" << artist
	<< "&api_key=" << APIKEY
	<< "&sk=" << _sessionid;

    curl_free(artist);
    curl_free(title);

    sig << "api_key" << APIKEY
	<< "artist" << song.getArtist()
	<< "method" << (unlove ? "track.unlove" : "track.love")
	<< "sk" << _sessionid
	<< "track" << song.getTitle()
	<< SECRET;

    std::string sighash(md5sum((char*)"%s", sig.str().c_str()));

    query << "&api_sig=" << sighash;

    OpenURL(GetServiceURL(), query.str().c_str());

    if(_response.find("<lfm status=\"ok\">") != std::string::npos) {
	iprintf("%s", "(Un)loved track successfully.");
	retval = true;
    }
    else if(_response.find("<lfm status=\"failed\">") != std::string::npos) {
	eprintf("%s%s", "Last.fm returned an error while (un)loving the currently playing track:\n", _response.c_str());
	if(CheckFailure(_response))
	    Failure();
    }

    CLEANUP();
    return retval;
}

bool CAudioScrobbler::SendNowPlaying(const Song& song)
{
    bool retval = false;

    char* artist = curl_easy_escape(_handle, song.getArtist().c_str(), 0);
    char* title = curl_easy_escape(_handle, song.getTitle().c_str(), 0);
    char* album = song.getAlbum().empty() ? 0 : curl_easy_escape(_handle, song.getAlbum().c_str(), 0);

    std::ostringstream query, sig;
    query << "method=track.updateNowPlaying&track=" << title
	<< "&artist=" << artist
	<< "&duration=" << song.getDuration()
	<< "&api_key=" << APIKEY
	<< "&sk=" << _sessionid;
    if(album) {
	query << "&album=" << album;
	sig << "album" << song.getAlbum();
    }

    curl_free(artist);
    curl_free(title);
    curl_free(album);

    sig << "api_key" << APIKEY
	<< "artist" << song.getArtist()
	<< "duration" << song.getDuration()
	<< "methodtrack.updateNowPlaying"
	<< "sk" << _sessionid
	<< "track" << song.getTitle()
	<< SECRET;

    std::string sighash(md5sum((char*)"%s", sig.str().c_str()));

    query << "&api_sig=" << sighash;

    OpenURL(GetServiceURL(), query.str().c_str());

    if(_response.find("<lfm status=\"ok\">") != std::string::npos) {
	iprintf("%s", "Updated \"Now Playing\" status successfully.");
	retval = true;
    }
    else if(_response.find("<lfm status=\"failed\">") != std::string::npos) {
	eprintf("%s%s", "Last.fm returned an error while updating the currently playing track:\n", _response.c_str());
	if(CheckFailure(_response))
	    Failure();
    }

    CLEANUP();
    return retval;
}

void CAudioScrobbler::Handshake()
{
    std::string username="";
    for(unsigned int i = 0; i < Config->getLUsername().length(); i++) {
	username.append(1, tolower(Config->getLUsername().c_str()[i]));
    }
    std::string authtoken(md5sum((char*)"%s%s", username.c_str(), Config->getLPassword().c_str()));

    std::ostringstream query, sig;
    query << "method=auth.getMobileSession&username=" << username << "&authToken=" << authtoken << "&api_key=" << APIKEY;

    sig << "api_key" << APIKEY << "authToken" << authtoken << "methodauth.getMobileSessionusername" << username << SECRET;
    std::string sighash(md5sum((char*)"%s", sig.str().c_str()));

    query << "&api_sig=" << sighash;

    OpenURL(GetServiceURL(), query.str().c_str());

    if(_response.find("<lfm status=\"ok\">") != std::string::npos) {
	size_t start, end;
	start = _response.find("<key>") + 5;
	end = _response.find("</key>");
	_sessionid = _response.substr(start, end-start);
	iprintf("%s%s", "Last.fm handshake successful. SessionID: ", _sessionid.c_str());
	_authed = true;
    }
    else if(_response.find("<lfm status=\"failed\">") != std::string::npos) {
	CheckFailure(_response);
	exit(EXIT_FAILURE);
    }

    CLEANUP();
}
