#include "mpdas.h"

CMPD* MPD = 0;

void
CMPD::SetSong(const Song *song)
{
    _cached = false;
	if(song && !(*song)["artist"].empty() && !(*song)["track"].empty()) {
        _song = *song;
		_gotsong = true;
		iprintf("New song: %s - %s", _song["artist"].c_str(), _song["track"].c_str());
        AudioScrobbler->SendNowPlaying(*song);
	}
	else {
		_gotsong = false;
	}
	_starttime = time(NULL);
}

void
CMPD::CheckSubmit(int curplaytime)
{
	if(!_gotsong || _cached || (_song["artist"].empty() || _song["track"].empty())) return;
	if(curplaytime - _start >= 240 || curplaytime - _start >= _song.duration()/2) {
		Cache->AddToCache(_song, _starttime);
		_cached = true;
	}
}

CMPD::CMPD()
{
    _conn = NULL;
	_gotsong = false;
	_connected = false;
	_cached = false;
    _songid = -1;
    _songpos = -1;

	if(Connect())
		iprintf("%s", "Connected to MPD.");
	else
		eprintf("%s", "Could not connect to MPD.");
}

CMPD::~CMPD()
{
	if(_conn)
		mpd_connection_free(_conn);
}

bool
CMPD::Connect()
{
    std::cout << _conn;
    if(_conn)
        mpd_connection_free(_conn);

	_conn = mpd_connection_new(Config->getMHost().c_str(), Config->getMPort(), 0);
    _connected = _conn && mpd_connection_get_error(_conn) == MPD_ERROR_SUCCESS;

    if(_connected && Config->getMPassword().size() > 0) {
        _connected &= mpd_run_password(_conn, Config->getMPassword().c_str());
    }

    if(_connected)
        mpd_run_subscribe(_conn, "mpdas");

	return _connected;
}

void
CMPD::GotNewSong(struct mpd_song *song)
{
    Song *s = new Song(song);
    SetSong(s);
    delete s;
}

void
CMPD::Update()
{
	if(!_connected) {
        iprintf("Reconnecting in 10 seconds.");
        sleep(10);
		if(Connect())
            iprintf("%s", "Reconnected!");
        else {
            eprintf("%s", "Could not reconnect.");
            return;
        }
	}

    mpd_status *status = mpd_run_status(_conn);
    mpd_stats *stats = mpd_run_stats(_conn);

    if(status && stats) {
        int newsongid = mpd_status_get_song_id(status);
        int newsongpos = mpd_status_get_elapsed_time(status);
        int curplaytime = mpd_stats_get_play_time(stats);
        mpd_song *song = mpd_run_current_song(_conn);

        // new song
        if(newsongid != _songid) {
            _songid = newsongid;
            _songpos = newsongpos;
            _start = curplaytime;

            if(song) {
                GotNewSong(song);
                mpd_song_free(song);
            }
        }

        // song playing
        if(newsongpos != _songpos) {
            _songpos = newsongpos;
            CheckSubmit(curplaytime);
        }

        // check for client-to-client messages
        if(mpd_send_read_messages(_conn)) {
            mpd_message *msg;
            while((msg = mpd_recv_message(_conn)) != NULL) {
                const char *text = mpd_message_get_text(msg);
                if(_gotsong && text && !strncmp(text, "love", 4))
                    AudioScrobbler->LoveTrack(_song);
                mpd_message_free(msg);
            }
            mpd_response_finish(_conn);
        }

        mpd_status_free(status);
        mpd_stats_free(stats);
    }
    else { // we have most likely lost our connection
        eprintf("Could not query MPD server: %s", mpd_connection_get_error_message(_conn));
        _connected = false;
    }
}
