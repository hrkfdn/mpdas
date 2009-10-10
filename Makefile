VERSION = 0.2.5

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
	@$(CXX) $(OBJ) $(LIBS) -o $(OUT)

clean:
	rm -rf $(OBJ) $(OUT)

install: all
	install mpdas ${PREFIX}/bin
	install -m 644 mpdas.1 ${MANPREFIX}/man/man1/mpdas.1

uninstall:
	-rm ${PREFIX}/bin/hrktorrent
	-rm ${MANPREFIX}/man/man1/mpdas.1
