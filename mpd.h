#ifndef _MPD_H
#define _MPD_H

typedef struct
{
	std::string artist, title, album;
	int time;
} song_t;

class CMPD
{
	public:
		CMPD();
		~CMPD();

		bool Connect();
		void Update();
		void SetSong(mpd_Song* song);
		void CheckSubmit();
		const song_t* GetSong() { return &_song; };

		inline bool isConnected() { return _connected; }
	private:
		static void StatusChanged(MpdObj*, ChangedStatusType);

		MpdObj* _obj;
		song_t _song;
		bool _gotsong;
		int _start;
		time_t _starttime;
		bool _connected;
		bool _cached;
};

extern CMPD* MPD;

#endif
