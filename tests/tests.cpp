#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../mpdas.h"

TEST_CASE("Song titles are correctly parsed", "[parseSongTitle]")
{
    const Song song("Test Artist", "Test Title", "Test Album", 100);
    std::string songArtist, songTitle;
    parseSongTitle(song, songArtist, songTitle);
    REQUIRE(songArtist == "Test Artist");
    REQUIRE(songTitle == "Test Title");
}
