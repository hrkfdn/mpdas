
AM_CPPFLAGS = -DCONFDIR="\"$(sysconfdir)\"" -DVERSION="\"$(VERSION)\""
AM_CXXFLAGS = -std=c++11

bin_PROGRAMS = mpdas

mpdas_SOURCES= \
    main.cpp md5.cpp utils.cpp mpd.cpp audioscrobbler.cpp cache.cpp \
    config.cpp

if DOUPNP
   mpdas_SOURCES += upnpav.cpp
endif

mpdas_LDADD = $(MPDAS_LIBS)


dist_man1_MANS = mpdas.1
