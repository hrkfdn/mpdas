#include "mpdas.h"

CCache* Cache = 0;

void
CCache::SaveCache()
{
	std::string path = getenv("HOME");
	path.append("/.mpdascache");
	remove(path.c_str());
	std::ofstream ofs(path.c_str());
	if(!_entries.size()) {
		remove(path.c_str());
		return;
	}

	for(unsigned int i = 0; i < _entries.size(); i++) {
		centry_t* entry = _entries[i];
		if(entry->album)
			ofs << true << "\n";
		ofs << entry->artist << "\n";
		ofs << entry->title << "\n";
		ofs << entry->time << "\n";
		ofs << entry->starttime;
		if(entry->album)
			ofs << "\n" << entry->album;
		if(i+1 == _entries.size())
			ofs.flush();
		else
			ofs << std::endl;
	}
	ofs.close();
}

void
CCache::LoadCache()
{
	int length;
	std::string path = getenv("HOME");
	path.append("/.mpdascache");
	std::ifstream ifs(path.c_str(), std::ios::in|std::ios::binary);

	ifs.seekg (0, std::ios::end);
	length = ifs.tellg();
	ifs.seekg (0, std::ios::beg);


	while(ifs.good()) {
		if(length == ifs.tellg())
			break;
		std::string artist, album, title;
		bool gotalbum = false;
		int time;
		time_t starttime;

		ifs >> gotalbum;
		ifs.ignore(1);
		ifs >> artist;
		ifs >> title;
		ifs >> time;
		ifs.ignore(1);
		ifs >> starttime;
		ifs.ignore(1);
		if(gotalbum)
			getline(ifs, album);
		AddToCache(time, artist, title, album, starttime, true);
	}

	ifs.close();
	remove(path.c_str());
}

void
CCache::WorkCache()
{
	if(_failtime && time(NULL) - _failtime < 300) {
		return;
	}
	_failtime = 0;
	while(_entries.size()) {
		if(AudioScrobbler->Scrobble(_entries.front())) {
			curl_free((void*)_entries.front()->artist);
			curl_free((void*)_entries.front()->title);
			if(_entries.front()->album)
				curl_free((void*)_entries.front()->album);
			delete _entries.front();
			_entries.erase(_entries.begin());
		}
		else {
			eprintf("%s", "Error scrobbling. Trying again in 5 minutes.");
			_failtime = time(NULL);
			break;
		}
		sleep(1);
	}
	SaveCache();
}

void
CCache::AddToCache(int time, std::string artist, std::string title, std::string album, time_t starttime, bool escaped = false)
{
	centry_t* entry = new centry_t;
	bzero(entry, sizeof(centry_t));

	entry->time = time;
	if(!escaped) {
		entry->artist = (char*)curl_escape(artist.c_str(), 0);
		entry->title = (char*)curl_escape(title.c_str(), 0);
		if(album.size())
			entry->album = (char*)curl_escape(album.c_str(), 0);
	} else {
		entry->artist = new char[artist.size()+1];
		strcpy(entry->artist, artist.c_str());
		entry->title = new char[title.size()+1];
		strcpy(entry->title, title.c_str());
		if(album.size()) {
			entry->album = new char[album.size()+1];
			strcpy(entry->album, album.c_str());
		}
	}
	entry->starttime = starttime;
	_entries.push_back(entry);
	SaveCache();
}
