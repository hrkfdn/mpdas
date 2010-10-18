#ifndef _AUDIOSCROBBLER_H
#define _AUDIOSCROBBLER_H


class CAudioScrobbler
{
	public:
		CAudioScrobbler();
		void Handshake();
		const std::string CreateScrobbleMessage(const int index, const centry_t* entry);
		const bool Scrobble(const centry_t* entry);
		void ReportResponse(const char* buf, const size_t size);
		const bool SendNowPlaying(const mpd_Song* song);
		void Failure();
		void GetLove();
	private:
		void InitPipe();
		void OpenURL(const std::string& url, const char* postfields, const char* errbuf);
		const bool CheckFailure(const std::string& response);

		CURL* _handle;

		std::string _password;
		std::string _response;

		std::string _sessionid;
		std::string _npurl;
		std::string _scroburl;

		bool _authed;
		int _failcount;
		int _ratingpipe;
		bool _love;
};

extern CAudioScrobbler* AudioScrobbler;

#endif
