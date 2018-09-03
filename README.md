# mpdas

[![Build Status](https://travis-ci.org/hrkfdn/mpdas.svg?branch=master)](https://travis-ci.org/hrkfdn/mpdas)

mpdas is a MPD AudioScrobbler client
supporting the 2.0 protocol specs.

It is written in C++ and uses libmpdclient
to retrieve the song data from MPD and
libcurl to post it to Last.fm

## Features

- Sets now-playing status
- Scrobbling (obviously)
- Caching
- Config files
- User switching
- "Love" tracks on Last.fm
    (e.g. with `mpc sendmessage mpdas love` and
     `mpc sendmessage mpdas unlove` to revert)

## Configuration

*NOTE*: There is a manpage available as well:
man mpdas

To configure mpdas, simply create a file called
.mpdasrc in your home directory or in
`$PREFIX/mpdasrc`. To change `$PREFIX`, modify the
Makefile.

Syntax is easy. Example:

```
username = lastfmuser
password = password
```

*NOTE*: In the past the password had to be an MD5
hash. As of v0.4.3 this has changed due to
Last.fm deprecating the previous authentication
method. Please supply the password in plain-text
from now on.

### Configuration options

- username: Last.FM username
- password: Last.FM password (plain-text)
- host: MPD Host
- mpdpassword: MPD Password
- port: MPD Port
- runas: Change the user mpdas runs as
- debug: Print debug information
- service: Will scrobble to Libre.fm if set to "librefm"
