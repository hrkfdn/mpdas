#include "mpdas.h"

#define ROOTURL		"http://ws.audioscrobbler.com/2.0/"
#define APIKEY		"a0ed2629d3d28606f67d7214c916788d"
#define	SECRET		"295f31c5d28215215b1503fb0327cc01"

CAudioScrobbler* AudioScrobbler = 0;

#define CLEANUP()	_response.clear()

size_t
writecb(void* ptr, size_t size, size_t nmemb, void *stream)
{
	AudioScrobbler->ReportResponse((char*)ptr, size*nmemb);
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

void
CAudioScrobbler::OpenURL(std::string url, const char* postfields = 0, char* errbuf = 0)
{
	curl_easy_setopt(_handle, CURLOPT_DNS_CACHE_TIMEOUT, 0);
	curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, writecb);

	if(postfields) {
		curl_easy_setopt(_handle, CURLOPT_POST, 1);
		curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, postfields);
	}
	else
		curl_easy_setopt(_handle, CURLOPT_POST, 0);
	if(errbuf)
		curl_easy_setopt(_handle, CURLOPT_ERRORBUFFER, errbuf);
	
	curl_easy_setopt(_handle, CURLOPT_URL, url.c_str());
	curl_easy_perform(_handle);
}


void
CAudioScrobbler::ReportResponse(char* buf, size_t size)
{
	_response.append(buf);
}

std::string
CAudioScrobbler::CreateScrobbleMessage(int index, centry_t* entry)
{
	std::ostringstream msg, sigmsg ;
	std::string artist, title, album, array = "=";

	char* temp = 0;
	temp = curl_easy_escape(_handle, entry->artist.c_str(), entry->artist.length());
	artist = temp;
	curl_free(temp);
	temp = curl_easy_escape(_handle, entry->title.c_str(), entry->title.length());
	title = temp;
	curl_free(temp);
	temp = curl_easy_escape(_handle, entry->album.c_str(), entry->album.length());
	album = temp;
	curl_free(temp);

	msg << "&album" << array << album;
	msg << "&api_key=" << APIKEY;
	msg << "&artist" << array << artist;
	msg << "&duration" << array << entry->time;
	msg << "&method=track.Scrobble";
	msg << "&timestamp" << array << entry->starttime;
	msg << "&track" << array << title;
	msg << "&sk=" << _sessionid;

	array = "";

	sigmsg << "album" << array << entry->album;
	sigmsg << "api_key" << APIKEY;
	sigmsg << "artist" << array << entry->artist;
	sigmsg << "duration" << array << entry->time;
	sigmsg << "methodtrack.Scrobble";
	sigmsg << "sk" << _sessionid;
	sigmsg << "timestamp" << array << entry->starttime;
	sigmsg << "track" << array << entry->title;
	sigmsg << SECRET;

	std::string sighash(md5sum((char*)"%s", sigmsg.str().c_str()));
	msg << "&api_sig=" << sighash;

	return msg.str();
}

void
CAudioScrobbler::Failure()
{
	_failcount += 1;
	if(_failcount >= 3) {
		eprintf("%s", "Re-Handshaking!");
		_failcount = 0;
		Handshake();
	}
}

bool
CAudioScrobbler::CheckFailure(std::string response)
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

bool
CAudioScrobbler::Scrobble(centry_t* entry)
{
	bool retval = false;
	if(!_authed) {
		eprintf("Handshake hasn't been done yet.");
		Handshake();
		return retval;
	}
	iprintf("Scrobbling: %s - %s", entry->artist.c_str(), entry->title.c_str());
	
	OpenURL(ROOTURL, CreateScrobbleMessage(0, entry).c_str());
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

bool
CAudioScrobbler::SendNowPlaying(mpd_Song* song)
{
	bool retval = false;
	if(!song || !song->artist || !song->title) return retval;

	char* artist = curl_easy_escape(_handle, song->artist, 0);
	char* title = curl_easy_escape(_handle, song->title, 0);
	char* album = 0;
	if(song->album)
		album = curl_easy_escape(_handle, song->album, 0);

	std::ostringstream query, sig;
	query << "method=track.updateNowPlaying&track=" << title << "&artist=" << artist << "&duration=" << song->time << "&api_key=" << APIKEY << "&sk=" << _sessionid;
	if(album) {
		query << "&album=" << album;
		sig << "album" << song->album;
	}

	sig << "api_key" << APIKEY << "artist" << song->artist << "duration" << song->time << "methodtrack.updateNowPlaying" << "sk" << _sessionid << "track" << song->title << SECRET;

	std::string sighash(md5sum((char*)"%s", sig.str().c_str()));

	query << "&api_sig=" << sighash;

	OpenURL(ROOTURL, query.str().c_str());

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

void
CAudioScrobbler::Handshake()
{
	std::string username="";
	for(unsigned int i = 0; i < Config->getLUsername().length(); i++) {
		username.append(1, tolower(Config->getLUsername().c_str()[i]));
	}
	std::string authtoken(md5sum((char*)"%s%s", username.c_str(), Config->getLPassword().c_str()));

	std::ostringstream query, sig;
	query << "method=auth.getMobileSession&username=" << Config->getLUsername() << "&authToken=" << authtoken << "&api_key=" << APIKEY;

	sig << "api_key" << APIKEY << "authToken" << authtoken << "methodauth.getMobileSessionusername" << Config->getLUsername() << SECRET;
	std::string sighash(md5sum((char*)"%s", sig.str().c_str()));

	query << "&api_sig=" << sighash;

	OpenURL(ROOTURL, query.str().c_str());

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
