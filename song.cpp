#include "song.hpp"


Song::Song(struct mpd_song *song)
{
    setTag("artist", mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
    setTag("track", mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
    setTag("album", mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
    setTag("trackNumber", mpd_song_get_tag(song, MPD_TAG_TRACK, 0));
    setTag("mbid", mpd_song_get_tag(song, MPD_TAG_MUSICBRAINZ_RELEASETRACKID, 0));
    setTag("albumArtist", mpd_song_get_tag(song, MPD_TAG_ALBUM_ARTIST, 0));

    setDuration(mpd_song_get_duration(song));
}

int Song::duration() const
{
    if((*this)["duration"].empty())
        return 0;

    return std::stoi((*this)["duration"]);
}

void Song::setTag(std::string name, const char* value)
{
    if(value != NULL)
        _tags[name] = value;
}

void Song::setDuration(int d)
{
    if(d > 0)
        _tags["duration"] = std::to_string(d);
}
