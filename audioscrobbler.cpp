#include "mpdas.h"
#include "http.hpp"

#define ROOTURL		"http://ws.audioscrobbler.com/2.0/"
#define APIKEY		"a0ed2629d3d28606f67d7214c916788d"
#define	SECRET		"295f31c5d28215215b1503fb0327cc01"

CAudioScrobbler* AudioScrobbler = 0;

CAudioScrobbler::CAudioScrobbler()
{
	_failcount = 0;
	_authed = false;
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

    params["api_sig"] = md5sum(sig.str());

    // Create a message in application/x-www-form-urlencoded format
    for(auto param : params)
        msg << "&" << param.first << "=" << conn.urlencode(param.second);

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
	start = conn.response().find("<error code=\"")+13;
	end = conn.response().find(">", start)-1;
	std::string errorcode = conn.response().substr(start, end-start);
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

	conn.post(ROOTURL, CreateScrobbleMessage(entry));
	if(conn.response().find("<lfm status=\"ok\">") != std::string::npos) {
		iprintf("%s", "Scrobbled successfully.");
		retval = true;
	}
	else if(conn.response().find("<lfm status=\"failed\">") != std::string::npos) {
		eprintf("%s%s", "Last.fm returned an error while scrobbling:\n", conn.response().c_str());
		if(CheckFailure(conn.response()))
			Failure();
	}

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

	conn.post(ROOTURL, CreateSignedMessage(params));

	if(conn.response().find("<lfm status=\"ok\">") != std::string::npos) {
		iprintf("%s", "Loved track successfully.");
		retval = true;
	}
	else if(conn.response().find("<lfm status=\"failed\">") != std::string::npos) {
		eprintf("%s%s", "Last.fm returned an error while loving the currently playing track:\n", conn.response().c_str());
		if(CheckFailure(conn.response()))
			Failure();
	}

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

	conn.post(ROOTURL, CreateSignedMessage(params));

	if(conn.response().find("<lfm status=\"ok\">") != std::string::npos) {
		iprintf("%s", "Updated \"Now Playing\" status successfully.");
		retval = true;
	}
	else if(conn.response().find("<lfm status=\"failed\">") != std::string::npos) {
		eprintf("%s%s", "Last.fm returned an error while updating the currently playing track:\n", conn.response().c_str());
		if(CheckFailure(conn.response()))
			Failure();
	}

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
	params["authToken"] = md5sum(username + Config->getLPassword());
	params["api_key"] = APIKEY;

        conn.post(ROOTURL, CreateSignedMessage(params));

	if(conn.response().find("<lfm status=\"ok\">") != std::string::npos) {
		size_t start, end;
		start = conn.response().find("<key>") + 5;
		end = conn.response().find("</key>");
		_sessionid = conn.response().substr(start, end-start);
		iprintf("%s%s", "Last.fm handshake successful. SessionID: ", _sessionid.c_str());
		_authed = true;
	}
	else if(conn.response().find("<lfm status=\"failed\">") != std::string::npos) {
		CheckFailure(conn.response());
		exit(EXIT_FAILURE);
	}

}
