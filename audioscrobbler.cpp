#include "mpdas.h"

#define HOST		"http://post.audioscrobbler.com"
#define PVERSION	"1.2.1"
#define CLIENT		"mp5"
#define CVERSION	"0.2.5"

CAudioScrobbler* AudioScrobbler = 0;

#define CLEANUP()	_response.clear()

size_t
writecb(void* ptr, size_t size, size_t nmemb, void *stream)
{
	AudioScrobbler->ReportResponse((char*)ptr, size*nmemb);
}

CAudioScrobbler::CAudioScrobbler()
{
	_ratingpipe = 0;
	_love = false;
	_failcount = 0;
	_authed = false;
	_response = "";
	_handle = curl_easy_init();
	if(!_handle) {
		eprintf("%s", "Could not initialize CURL.");
		exit(EXIT_FAILURE);
	}

	InitPipe();
}

void
CAudioScrobbler::InitPipe()
{
	umask(0666);
	if(mkfifo("/tmp/mpdaspipe", 0666) != 0 && errno != EEXIST)
		eprintf("Could not create the rating pipe. (%s)", strerror(errno));
	else {
		chmod("/tmp/mpdaspipe", 0666); // somehow this didnt suffice in the mkfifo() call
		_ratingpipe = open("/tmp/mpdaspipe", O_RDONLY | O_NDELAY);
		if(!_ratingpipe)
			eprintf("Could not open the rating pipe. (%s)", strerror(errno));
	}
}


void
CAudioScrobbler::OpenURL(const std::string& url, const char* postfields = 0, const char* errbuf = 0)
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
CAudioScrobbler::ReportResponse(const char* buf, const size_t size)
{
	_response.append(buf);
}

const std::string
CAudioScrobbler::CreateScrobbleMessage(const int index, const centry_t* entry)
{
	std::stringstream msg;
	msg << "&a[" << index << "]=" << entry->artist;
	msg << "&t[" << index << "]=" << entry->title;
	msg << "&i[" << index << "]=" << entry->starttime;
	msg << "&o[" << index << "]=P";
	msg << "&r[" << index << "]=";
	if(_love)
		msg << "L";
	msg << "&l[" << index << "]=" << entry->time;
	msg << "&b[" << index << "]=";
	if(entry->album)
		msg << entry->album;
	msg << "&n[" << index << "]=&m[" << index << "]=";

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

const bool
CAudioScrobbler::CheckFailure(const std::string& response)
{
	bool retval = false;
	if(_response.find("BADSESSION")) {
		eprintf("%s", "Bad session ID, re-handshaking!");
		Handshake();
		retval = false;
	}
	else if(_response.find("FAILED"))
		retval = true;
	else if(_response.find("OK"))
		retval = false;
	return retval;
}

void
CAudioScrobbler::GetLove()
{
	if(_ratingpipe == 0)
		return;
	char buf[80];
	int numread = 0;
	memset(buf, 0, sizeof(buf));

	numread = read(_ratingpipe, &buf, sizeof(buf));
	if(numread > 0) {
		if(strstr(buf, "L")) {
			iprintf("Track will be scrobbled with \"love\" attribute.");
			_love = true;
		}
		else if(strstr(buf, "C")) {
			iprintf("Track will not be scrobbled with \"love\" attribute.");
			_love = false;
		}
	}
}

const bool
CAudioScrobbler::Scrobble(const centry_t* entry)
{
	bool retval = false;
	if(!_authed) {
		eprintf("Handshake hasn't been done yet.");
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
	else {
		if(CheckFailure(_response))
			Failure();
	}

	CLEANUP();

	_love = false;
	return retval;
}

const bool
CAudioScrobbler::SendNowPlaying(const mpd_Song* song)
{
	bool retval = false;
	if(!song || !song->artist || !song->title) return retval;
	_love = false;

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
	else {
		if(CheckFailure(_response))
			Failure();
	}
	CLEANUP();
	return retval;
}

void
CAudioScrobbler::Handshake()
{
	time_t timestamp = time(NULL);
	std::string authtoken(md5sum((char*)"%s%i", Config->getLPassword().c_str(), timestamp));

	std::ostringstream query;
	query << HOST << "/?hs=true&p=" << PVERSION << "&c=" << CLIENT << "&v=" << CVERSION << "&u=" << Config->getLUsername() << "&t=" << timestamp << "&a=" << authtoken;
	
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
	else if(_response.find("BADTIME") == 0) {
		eprintf("%s", "Your computer time is not accurate enough to generate a session id.");
		exit(EXIT_FAILURE);
	}

	CLEANUP();
}
