#ifndef _MPD_H
#define _MPD_H

class Song {
    public:
        Song() {};
        Song(struct mpd_song *song);
        Song(std::string artist, std::string title, std::string album, int duration) {
            this->artist = artist;
            this->title = title;
            this->album = album;
            this->duration = duration;
        }

        std::string getArtist() const { return artist; }
        std::string getTitle() const { return title; }
        std::string getAlbum() const { return album; }
        int getDuration() const { return duration; }

        bool operator != (const Song &other) const {
            return this->getArtist() != other.getArtist() or
                this->getAlbum() != other.getAlbum() or
                this->getTitle() != other.getTitle();
        }
    private:
        std::string artist, title, album;
        int duration;
};

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
