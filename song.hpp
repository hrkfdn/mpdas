#ifndef SONG_HPP
#define SONG_HPP

#include <map>
#include <string>
#include <mpd/client.h>

class Song {
    public:
        Song() {};
        Song(struct mpd_song *song);

        const std::map<std::string, std::string>& tags() const {
            return _tags;
        }

        int duration() const;

        // Do not create tag if it does not exist
        std::string operator[] (std::string tagName) const {
            if(_tags.count(tagName))
                return _tags.at(tagName);
            else
                return std::string();
        }
        std::string& operator[] (std::string tagName) {
            return _tags[tagName];
        }

    private:
        std::map<std::string, std::string> _tags;

        void setTag(std::string name, const char* value);
        void setDuration(int d);
};

#endif
