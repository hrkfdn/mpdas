VERSION = 0.3.1

CXX	?= g++
OBJ	= main.o md5.o utils.o mpd.o audioscrobbler.o cache.o config.o
OUT	= mpdas
PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/man/man1
CONFIG ?= $(PREFIX)/etc

CXXFLAGS	+= `pkg-config --cflags libmpd libcurl` 
LIBS		= `pkg-config --libs libmpd libcurl`

CXXFLAGS	+= -DCONFDIR="\"$(CONFIG)\"" -DVERSION="\"$(VERSION)\""


all: $(OUT)

.cpp.o:
	@echo [CXX] $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT): $(OBJ)
	@echo [LD] $@
	@$(CXX) $(LDFLAGS) $(OBJ) $(LIBS) -o $(OUT)

clean:
	rm -rf $(OBJ) $(OUT)

install: all
	install -Dm755 mpdas ${DESTDIR}${PREFIX}/bin/mpdas
	install -Dm644 mpdas.1 ${DESTDIR}${MANPREFIX}/mpdas.1

uninstall:
	-rm ${DESTDIR}${PREFIX}/bin/mpdas
	-rm ${DESTDIR}${MANPREFIX}/mpdas.1
