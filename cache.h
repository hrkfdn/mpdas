#ifndef _CACHE_H
#define _CACHE_H

class CacheEntry
{
    public:
        CacheEntry() {};
        CacheEntry(const Song& song, time_t starttime) {
            this->song = song;
            this->starttime = starttime;
        }

        // stream serialization
        friend std::ofstream& operator <<(std::ofstream& outstream, const CacheEntry& inobj);
        friend std::ifstream& operator >>(std::ifstream& instream, CacheEntry& outobj);

        Song getSong() const { return song; }
        time_t getStartTime() const { return starttime; }
    private:
        Song song;
        time_t starttime;
};

class CCache
{
    public:
        CCache() { _failtime = 0; }
        void AddToCache(const Song& song, time_t starttime);
        void WorkCache();
        void SaveCache();
        void LoadCache();
    private:
        time_t _failtime;
        std::vector<CacheEntry*> _entries;
};

extern CCache* Cache;

#endif
