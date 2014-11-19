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
        CacheEntry* entry = _entries[i];
        ofs << *entry;

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

        CacheEntry *entry = new CacheEntry();
        ifs >> *entry;
        _entries.push_back(entry);
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
        if(AudioScrobbler->Scrobble(*_entries.front())) {
            delete _entries.front();
            _entries.erase(_entries.begin());
        }
        else {
            eprintf("%s", "Error scrobbling. Trying again in 5 minutes.");
            _failtime = time(NULL);
            AudioScrobbler->Failure();
            break;
        }
        sleep(1);
    }
    SaveCache();
}

void
CCache::AddToCache(const Song& song, time_t starttime)
{
    CacheEntry *entry = new CacheEntry(song, starttime);

    _entries.push_back(entry);
    SaveCache();
}

std::ofstream& operator <<(std::ofstream& outstream, const CacheEntry& inobj)
{
    Song song = inobj.getSong();
    outstream << song.getArtist() << std::endl
        << song.getTitle() << std::endl
        << song.getAlbum() << std::endl
        << song.getDuration() << std::endl
        << inobj.getStartTime();

    return outstream;
}

std::ifstream& operator >>(std::ifstream& instream, CacheEntry& outobj)
{
    std::string artist, title, album;
    int duration;
    time_t starttime;

    getline(instream, artist);
    getline(instream, title);
    getline(instream, album);

    instream >> duration;
    instream.ignore(1);
    instream >> starttime;
    instream.ignore(1);

    Song song(artist, title, album, duration);
    outobj = CacheEntry(song, starttime);

    return instream;
}
