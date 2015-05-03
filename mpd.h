#ifndef _MPD_H
#define _MPD_H


class CMPD
{
	public:
		CMPD();
		~CMPD();

		bool Connect();
		void Update();
		void SetSong(const Song *song);
		void CheckSubmit(int curplaytime);
		Song GetSong() const { return _song; };

		inline bool isConnected() { return _connected; }
	private:
        void GotNewSong(struct mpd_song *song);

		mpd_connection *_conn;
        int _songid;
        int _songpos;
		Song _song;
		bool _gotsong;
		int _start;
		time_t _starttime;
		bool _connected;
		bool _cached;
};

extern CMPD* MPD;

#endif
