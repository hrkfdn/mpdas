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
CAudioScrobbler::CreateSignedMessage(std::map<std::string, std::string> params)
{
    std::ostringstream msg, sig;

    // Add the Last.fm method signature: http://www.last.fm/api/authspec#8
    // std::map keeps the elements sorted by key, which makes this simple
    for(auto param : params)
        sig << param.first << param.second;
    sig << SECRET;

    params["api_sig"] = md5sum((char*)"%s", sig.str().c_str());

    // Create a message in application/x-www-form-urlencoded format
    for(auto param : params)
    {
        char* temp = curl_easy_escape(_handle, param.second.c_str(), 0);
        msg << "&" << param.first << "=" << std::string(temp);
        curl_free(temp);
    }

    return msg.str();
}

std::string
CAudioScrobbler::CreateScrobbleMessage(const CacheEntry& entry)
{
    std::map<std::string, std::string> params = entry.getSong().tags();

    params["method"] = "track.scrobble";
    params["timestamp"] = std::to_string(entry.getStartTime());
    params["api_key"] = APIKEY;
    params["sk"] = _sessionid;

    return CreateSignedMessage(params);
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
CAudioScrobbler::Scrobble(const CacheEntry& entry)
{
	bool retval = false;
	if(!_authed) {
		eprintf("Handshake hasn't been done yet.");
		Handshake();
		return retval;
	}
	iprintf("Scrobbling: %s - %s", entry.getSong()["artist"].c_str(), entry.getSong()["track"].c_str());

	OpenURL(ROOTURL, CreateScrobbleMessage(entry).c_str());
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
CAudioScrobbler::LoveTrack(const Song& song)
{
    bool retval = false;

    std::map<std::string, std::string> params;

    params["artist"] = song["artist"];
    params["track"] = song["track"];
    params["method"] = "track.love";
    params["api_key"] = APIKEY;
    params["sk"] = _sessionid;

	OpenURL(ROOTURL, CreateSignedMessage(params).c_str());

	if(_response.find("<lfm status=\"ok\">") != std::string::npos) {
		iprintf("%s", "Loved track successfully.");
		retval = true;
	}
	else if(_response.find("<lfm status=\"failed\">") != std::string::npos) {
		eprintf("%s%s", "Last.fm returned an error while loving the currently playing track:\n", _response.c_str());
		if(CheckFailure(_response))
			Failure();
	}

	CLEANUP();
	return retval;
}

bool
CAudioScrobbler::SendNowPlaying(const Song& song)
{
	bool retval = false;

	std::map<std::string, std::string> params = song.tags();

	params["method"] = "track.updateNowPlaying";
	params["api_key"] = APIKEY;
	params["sk"] = _sessionid;

	OpenURL(ROOTURL, CreateSignedMessage(params).c_str());

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

// This method uses the DEPRECATED authToken parameter
void
CAudioScrobbler::Handshake()
{
	std::map<std::string, std::string> params;

	std::string username="";
	for(unsigned int i = 0; i < Config->getLUsername().length(); i++) {
		username.append(1, tolower(Config->getLUsername().c_str()[i]));
	}

	params["method"] = "auth.getMobileSession";
	params["username"] = Config->getLUsername();
	params["authToken"] = md5sum((char*)"%s%s", username.c_str(), Config->getLPassword().c_str());
	params["api_key"] = APIKEY;

	OpenURL(ROOTURL, CreateSignedMessage(params).c_str());

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
