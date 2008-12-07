#ifndef _AUDIOSCROBBLER_H
#define _AUDIOSCROBBLER_H


class CAudioScrobbler
{
	public:
		CAudioScrobbler();
		void Handshake();
		std::string CreateScrobbleMessage(int index, centry_t* entry);
		bool Scrobble(centry_t* entry);
		void ReportResponse(char* buf, size_t size);
		bool SendNowPlaying(mpd_Song* song);
		void Failure();
		void GetLove();
	private:
		void OpenURL(std::string url, const char* postfields, char* errbuf);
		bool CheckFailure(std::string response);

		CURL* _handle;

		std::string _password;
		std::string _response;

		std::string _sessionid;
		std::string _npurl;
		std::string _scroburl;

		bool _authed;
		int _failcount;
		FILE* _ratingpipe;
		bool _love;
};

extern CAudioScrobbler* AudioScrobbler;

#endif
