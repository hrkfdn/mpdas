#ifndef _MPD_H
#define _MPD_H

class Song {
public:
	Song() {};
	Song(struct mpd_song *song);
  Song(std::string artist, std::string title, std::string album, int duration, std::string track, std::string mbid) {
		this->artist = artist;
		this->title = title;
		this->album = album;
		this->duration = duration;
		this->track = track;
		this->mbid = mbid;
	}

	std::string getArtist() const { return artist; }
	std::string getTitle() const { return title; }
	std::string getAlbum() const { return album; }
	std::string getAlbumArtist() const { return albumartist; }
	int getDuration() const { return duration; }
        std::string getTrack() const { return track; }
	std::string getMusicBrainzId() const { return mbid; }
private:
        std::string albumartist, artist, title, album, track,mbid;
	int duration = -1;
};

class CMPD
{
public:
	CMPD(CConfig *cfg);
	~CMPD();

	bool Connect();
	void Update();
	void SetSong(const Song *song);
	void CheckSubmit(int curplaytime);
	Song GetSong() const { return _song; };

	inline bool isConnected() { return _connected; }
private:
	void GotNewSong(struct mpd_song *song);

	CConfig *_cfg;
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
