#ifndef _UPNPAV_H
#define _UPNPAV_H

#include "mpd.h" // For Song

class CUPNP
{
public:
	CUPNP();
	~CUPNP();

	void Update();
	bool isConnected();

private:
	class Internal;
	Internal *m;
	void GotNewSong(struct mpd_song *song);
	void SetSong(const Song *song);
	bool Connect();
	void CheckSubmit(int curplaytime);
	Song GetSong();
};

extern CUPNP* UPNP;

#endif
