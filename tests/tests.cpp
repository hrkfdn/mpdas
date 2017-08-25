#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../mpdas.h"

TEST_CASE("Song titles are correctly parsed", "[parseSongTitle]")
{
    std::string songArtist, songTitle;
    SECTION("Regular song")
    {
        const Song song1("Test Artist", "Test Title", "Test Album", 100);
        parseSongTitle(song1, songArtist, songTitle);
        REQUIRE(songArtist == "Test Artist");
        REQUIRE(songTitle == "Test Title");

        const Song song2("Test-Artist", "Test Title", "Test Album", 100);
        parseSongTitle(song2, songArtist, songTitle);
        REQUIRE(songArtist == "Test-Artist");
        REQUIRE(songTitle == "Test Title");
    }

    SECTION("Song with no artist and well-formed title")
    {
        const Song song1("", "Test Artist - Test Title", "", 100);
        parseSongTitle(song1, songArtist, songTitle);
        REQUIRE(songArtist == "Test Artist");
        REQUIRE(songTitle == "Test Title");

        const Song song2("", "Test-Artist - Test Title", "", 100);
        parseSongTitle(song2, songArtist, songTitle);
        REQUIRE(songArtist == "Test-Artist");
        REQUIRE(songTitle == "Test Title");

        const Song song3("", "Test-Artist - Test - Title", "", 100);
        parseSongTitle(song3, songArtist, songTitle);
        REQUIRE(songArtist == "Test-Artist");
        REQUIRE(songTitle == "Test - Title");

        const Song song4("", "Test-Artist - Test-Title", "", 100);
        parseSongTitle(song4, songArtist, songTitle);
        REQUIRE(songArtist == "Test-Artist");
        REQUIRE(songTitle == "Test-Title");

        const Song song5("", " - Test Title", "", 100);
        parseSongTitle(song5, songArtist, songTitle);
        REQUIRE(songArtist == "");
        REQUIRE(songTitle == "Test Title");
    }

    SECTION("Song with no artist and malformed title")
    {
        const Song song1("", "Test Title", "", 100);
        parseSongTitle(song1, songArtist, songTitle);
        REQUIRE(songArtist == "");
        REQUIRE(songTitle == "Test Title");

        const Song song2("", "- Test Title", "", 100);
        parseSongTitle(song2, songArtist, songTitle);
        REQUIRE(songArtist == "");
        REQUIRE(songTitle == "- Test Title");
    }
}
