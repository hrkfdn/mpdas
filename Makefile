VERSION = 0.4.5

CXX	?= g++
OBJ	= main.o md5.o utils.o mpd.o audioscrobbler.o cache.o config.o ini.o
OUT	= mpdas
PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/man/man1
CONFIG ?= $(PREFIX)/etc

CXXFLAGS	+= `pkg-config --cflags libmpdclient libcurl`
LIBS		= `pkg-config --libs libmpdclient libcurl`

CXXFLAGS	+= -DCONFDIR="\"$(CONFIG)\"" -DVERSION="\"$(VERSION)\""


all: $(OUT)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LIBS) -o $(OUT)

clean:
	rm -rf $(OBJ) $(OUT)

install: all
	install -d ${DESTDIR}${PREFIX}/bin
	install -d ${DESTDIR}${MANPREFIX}
	install -m 755 mpdas ${DESTDIR}${PREFIX}/bin
	install -m 644 mpdas.1 ${DESTDIR}${MANPREFIX}/mpdas.1

uninstall:
	-rm ${DESTDIR}${PREFIX}/bin/mpdas
	-rm ${DESTDIR}${MANPREFIX}/mpdas.1
