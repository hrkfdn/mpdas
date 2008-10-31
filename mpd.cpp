#include "mpdas.h"

CMPD* MPD = 0;


void
CMPD::SetSong(mpd_Song* song)
{
	_cached = false;
	if(song && song->artist && song->title) {
		_song.artist = song->artist;
		_song.title = song->title;
		if(song->album)
			_song.album = song->album;
		else
			_song.album = "";
		_song.time = song->time;
		_gotsong = true;
		iprintf("New song: %s - %s", _song.artist.c_str(), _song.title.c_str());
	}
	else {
		_gotsong = false;
		return;
	}
	_start = mpd_stats_get_playtime(_obj);
	_starttime = time(NULL);
	AudioScrobbler->SendNowPlaying(song);
}

void
CMPD::CheckSubmit()
{
	if(!_gotsong || _cached || (!_song.artist.size() || !_song.title.size())) return;
	int curplaytime = mpd_stats_get_playtime(_obj);
	if(curplaytime - _start >= 240 || curplaytime - _start >= _song.time/2) {
		Cache->AddToCache(_song.time, _song.artist, _song.title, _song.album, _starttime, false);
		_cached = true;
	}
}

void
CMPD::StatusChanged(MpdObj* obj, ChangedStatusType what)
{
	mpd_Song* song = 0;

	if(what & MPD_CST_SONGID) {
		song = mpd_playlist_get_current_song(obj);
		if(song)
			MPD->SetSong(song);
	}
	else if(what & MPD_CST_STATE) {
		if(mpd_player_get_state(obj) == MPD_PLAYER_STOP)
			MPD->SetSong(0);
	}
	else if(what & MPD_CST_ELAPSED_TIME) {
		MPD->CheckSubmit();
	}
}

CMPD::CMPD()
{
	_gotsong = false;
	_connected = false;
	_cached = false;
	_obj = mpd_new((char*)Config->getMHost().c_str(), Config->getMPort(), (char*)Config->getMPassword().c_str());
	mpd_signal_connect_status_changed(_obj, (StatusChangedCallback)&StatusChanged, NULL);
	if(Connect())
		iprintf("%s", "Connected to MPD.");
	else
		eprintf("%s", "Could not connect to MPD.");
}

CMPD::~CMPD()
{
	if(_obj)
		mpd_free(_obj);
}

bool
CMPD::Connect()
{
	_connected = true;
	if(mpd_connect(_obj)) {
		_connected = false;
		return _connected;
	}
	if(mpd_send_password(_obj))
		_connected = false;
	return (_connected == true);
}


void
CMPD::Update()
{
	if(!_connected || !mpd_check_connected(_obj)) {
		eprintf("%s", "Disconnected from MPD. Reconnecting ..");
		Connect();
		return;
	}
	mpd_status_update(_obj);
	mpd_stats_update(_obj);
}
