#ifndef _AUDIOSCROBBLER_H
#define _AUDIOSCROBBLER_H


class CAudioScrobbler
{
	public:
		CAudioScrobbler();
        ~CAudioScrobbler();

		void Handshake();
		std::string CreateScrobbleMessage(int index, const CacheEntry& entry);
		bool Scrobble(const CacheEntry& entry);
		void ReportResponse(char* buf, size_t size);
        bool LoveTrack(const Song& song, bool unlove = false);
		bool SendNowPlaying(const Song& song);
		void Failure();
	private:
		std::string GetServiceURL();
		void OpenURL(std::string url, const char* postfields, char* errbuf);
		bool CheckFailure(std::string response);

		CURL* _handle;

		std::string _password;
		std::string _response;

		std::string _sessionid;

		bool _authed;
		int _failcount;
};

extern CAudioScrobbler* AudioScrobbler;

#endif
