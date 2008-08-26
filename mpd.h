#ifndef _MPD_H
#define _MPD_H

class CMPD
{
	public:
		CMPD();
		~CMPD();

		bool Connect();
		void Update();
		void SetSong(mpd_Song* song);
		void CheckSubmit();

		inline bool isConnected() { return _connected; }
	private:
		static void StatusChanged(MpdObj*, ChangedStatusType);

		MpdObj* _obj;
		mpd_Song _song;
		int _start;
		time_t _starttime;
		bool _connected;
		bool _cached;
};

extern CMPD* MPD;

#endif
