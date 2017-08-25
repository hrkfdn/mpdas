#ifndef _UTILS_H
#define _UTILS_H

bool fileexists(const char* file);
void iprintf(const char* fmt, ...);
void eprintf(const char* fmt, ...);
std::string md5sum(const char* fmt, ...);
std::string timestr();
void parseSongTitle(const Song &song, std::string &songArtist, std::string &songTitle);

#endif
