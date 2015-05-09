#ifndef _AUDIOSCROBBLER_H
#define _AUDIOSCROBBLER_H

#include <map>
#include "http.hpp"

class CAudioScrobbler
{
	public:
		CAudioScrobbler();

		void Handshake();
		std::string CreateScrobbleMessage(const CacheEntry& entry);
		bool Scrobble(const CacheEntry& entry);
        bool LoveTrack(const Song& song);
		bool SendNowPlaying(const Song& song);
		void Failure();
	private:
		bool CheckFailure(std::string response);
        std::string CreateSignedMessage(std::map<std::string, std::string> params);


		std::string _response;

		std::string _sessionid;

		bool _authed;
		int _failcount;
                Http conn;
};

extern CAudioScrobbler* AudioScrobbler;

#endif
