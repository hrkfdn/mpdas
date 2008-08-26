#include "mpdas.h"

#define HOST		"http://post.audioscrobbler.com"
#define VERSION		"1.2.1"
#define CLIENT		"mp5"
#define CVERSION	"0.1"

CAudioScrobbler* AudioScrobbler = 0;

#define CLEANUP()	_response.clear()

size_t
writecb(void* ptr, size_t size, size_t nmemb, void *stream)
{
	AudioScrobbler->ReportResponse((char*)ptr, size*nmemb);
}

CAudioScrobbler::CAudioScrobbler()
{
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
	std::stringstream msg;
	msg << "&a[" << index << "]=" << entry->artist;
	msg << "&t[" << index << "]=" << entry->title;
	msg << "&i[" << index << "]=" << entry->starttime;
	msg << "&o[" << index << "]=P&r[" << index << "]=";
	msg << "&l[" << index << "]=" << entry->time;
	msg << "&b[" << index << "]=";
	if(entry->album)
		msg << entry->album;
	msg << "&n[" << index << "]=&m[" << index << "]=";

	return msg.str();
}

bool
CAudioScrobbler::Scrobble(centry_t* entry)
{
	bool retval = false;
	if(!_authed) {
		eprintf("Handshake hasn't been done that.");
		Handshake();
		return retval;
	}
	std::ostringstream post;
	iprintf("Scrobbling: %s - %s", entry->artist, entry->title);
	post << "s=" << _sessionid;
	post << CreateScrobbleMessage(0, entry);

	OpenURL(_scroburl, post.str().c_str());
	if(_response.find("OK") == 0)
		retval = true;
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

	std::ostringstream post;
	post << "s=" << _sessionid << "&a=" << artist << "&t=" << title << "&b=";
	if(album)
		post << album;
	post << "&l=" << song->time << "&n=&m=";

	OpenURL(_npurl, post.str().c_str());

	if(_response.find("OK") == 0)
		retval = true;
	CLEANUP();
	return retval;
}

void
CAudioScrobbler::Handshake()
{
	time_t timestamp = time(NULL);
	_password = md5sum((char*)"%s", (char*)LPASSWORD);
	std::string authtoken(md5sum((char*)"%s%i", _password.c_str(), timestamp));

	std::ostringstream query;
	query << HOST << "/?hs=true&p=" << VERSION << "&c=" << CLIENT << "&v=" << CVERSION << "&u=" << LUSER << "&t=" << timestamp << "&a=" << authtoken;
	
	OpenURL(query.str());
	if(_response.find("OK") == 0) {
		iprintf("%s", "AudioScrobbler handshake successful.");
		_authed = true;
		_sessionid = _response.substr(3, 32);
		int found = _response.find("\n", 36);
		_npurl = _response.substr(36, found-36);
		_scroburl = _response.substr(found+1, _response.length()-found-2);
	}
	else if(_response.find("BADAUTH") == 0) {
		eprintf("%s", "Bad username/password.");
		exit(EXIT_FAILURE);
	}

	CLEANUP();
}
