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
	private:
		void OpenURL(std::string url, const char* postfields, char* errbuf);

		CURL* _handle;

		std::string _password;
		std::string _response;

		std::string _sessionid;
		std::string _npurl;
		std::string _scroburl;
};

extern CAudioScrobbler* AudioScrobbler;

#endif
