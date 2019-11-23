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

CAudioScrobbler::CAudioScrobbler(CConfig *cfg)
{
	_cfg = cfg;
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
	if(_cfg->getService() == LibreFm) {
		return "https://libre.fm/2.0/";
	}
	return "https://ws.audioscrobbler.com/2.0/";
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
		eprintf("libcurl error (%d): %s", res, curl_easy_strerror(res));
		eprintf("Will retry %d times with a %d second delay.", CURL_MAX_RETRIES, CURL_RETRY_DELAY);

		int retries = 0;
		do {
			sleep(CURL_RETRY_DELAY);
			retries++;
			eprintf("Retry %d/%d", retries, CURL_MAX_RETRIES);

			res = curl_easy_perform(_handle);
			if (res != CURLE_OK) {
				eprintf("Failed: %s", curl_easy_strerror(res));
			}
		} while (res != CURLE_OK && retries < CURL_MAX_RETRIES);
	}
}


void CAudioScrobbler::ReportResponse(char* buf, size_t size)
{
	_response.append(buf);
}

std::string CAudioScrobbler::CreateScrobbleMessage(int index, const CacheEntry& entry)
{
	const Song& song = entry.getSong();

	CLastFMMessage msg(_handle);
	msg.AddField("method", "track.Scrobble");
	msg.AddField("artist", song.getArtist());
	msg.AddField("track", song.getTitle());
	msg.AddField("duration", song.getDuration());
	msg.AddField("timestamp", entry.getStartTime());
	msg.AddField("sk", _sessionid);
	msg.AddField("api_key", APIKEY);

	if(!song.getAlbum().empty()) {
		msg.AddField("album", song.getAlbum());
	}

	if(!song.getAlbumArtist().empty()) {
		msg.AddField("albumArtist", song.getAlbumArtist());
	}

	if(!song.getMusicBrainzId().empty()) {
		msg.AddField("mbid", song.getMusicBrainzId());
	}

	if(!song.getTrack() == -1) {
		msg.AddField("trackNumber", song.getTrack());
	}

	return msg.GetMessage();
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
	case 13:
		eprintf("Invalid method signature.");
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

	CLastFMMessage msg(_handle);
	msg.AddField("method", unlove ? "track.unlove" : "track.love");
	msg.AddField("artist", song.getArtist());
	msg.AddField("track", song.getTitle());
	msg.AddField("api_key", APIKEY);
	msg.AddField("sk", _sessionid);

	OpenURL(GetServiceURL(), msg.GetMessage().c_str());

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

	CLastFMMessage msg(_handle);
	msg.AddField("method", "track.updateNowPlaying");
	msg.AddField("artist", song.getArtist());
	msg.AddField("track", song.getTitle());
	msg.AddField("duration", song.getDuration());
	msg.AddField("sk", _sessionid);
	msg.AddField("api_key", APIKEY);

	if(!song.getAlbum().empty()) {
		msg.AddField("album", song.getAlbum());
	}

	if(!song.getAlbumArtist().empty()) {
		msg.AddField("albumArtist", song.getAlbumArtist());
	}

	if(!song.getMusicBrainzId().empty()) {
		msg.AddField("mbid", song.getMusicBrainzId());
	}

	if(!song.getTrack() == -1) {
		msg.AddField("trackNumber", song.getTrack());
	}

	OpenURL(GetServiceURL(), msg.GetMessage().c_str());

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
	std::string username = "";
	for(unsigned int i = 0; i < _cfg->Get("username").length(); i++) {
		username.append(1, tolower(_cfg->Get("username").c_str()[i]));
	}
	std::string password = _cfg->Get("password");

	CLastFMMessage msg(_handle);

	msg.AddField("method", "auth.getMobileSession");
	msg.AddField("username", username);

	if(_cfg->getService() == LastFm) {
		msg.AddField("password", password);
	}
	else {
		std::string password_hashed(md5sum((char*)"%s", password.c_str()));
		std::string authtoken(md5sum((char*)"%s%s", username.c_str(), password_hashed.c_str()));
		msg.AddField("authToken", authtoken);
		msg.AddField("password", password_hashed);
	}
	msg.AddField("api_key", APIKEY);

	OpenURL(GetServiceURL(), msg.GetMessage().c_str());

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

std::string CLastFMMessage::GetMessage()
{
	std::ostringstream strstream;
	for(std::map<std::string, std::string>::iterator it = valueMap.begin(); it != valueMap.end(); ++it) {
		if(it != valueMap.begin()) {
			strstream << "&";
		}

		char* escaped = curl_easy_escape(curl_handle, it->second.c_str(), it->second.length());
		strstream << it->first << "=" << escaped;
		curl_free(escaped);
	}

	// append signature hash
	strstream << "&api_sig=" << GetSignatureHash();

	return strstream.str();
}

std::string CLastFMMessage::GetSignatureHash()
{
	std::ostringstream strstream;
	for(std::map<std::string, std::string>::iterator it = valueMap.begin(); it != valueMap.end(); ++it) {
		strstream << it->first << it->second;
	}

	// append secret key
	strstream << SECRET;

	return std::string(md5sum((char*)"%s", strstream.str().c_str()));
}
