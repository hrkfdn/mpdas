#ifndef _CACHE_H
#define _CACHE_H

typedef struct
centry_s
{
	time_t starttime;

	std::string artist;
	std::string title;
	std::string album;
	int time;
} centry_t;

class CCache
{
	public:
		CCache() { _failtime = 0; }
		void AddToCache(int time, std::string& artist, std::string& title, std::string& album, time_t starttime);
		void WorkCache();
		void SaveCache();
		void LoadCache();
	private:
		time_t _failtime;
		std::vector<centry_s*> _entries;
};

extern CCache* Cache;

#endif
