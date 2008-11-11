CXX	?= g++
OBJ	= main.o md5.o utils.o mpd.o audioscrobbler.o cache.o config.o
OUT	= mpdas
PREFIX = /usr/local

CXXFLAGS	+= `pkg-config --cflags libmpd libcurl`
LIBS		= `pkg-config --libs libmpd libcurl`

all: $(OUT)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT): $(OBJ)
	$(CXX) $(LIBS) $(OBJ) -o $(OUT)

clean:
	rm -rf $(OBJ) $(OUT)

install: all
	install mpdas ${PREFIX}/bin

uninstall:
	-rm ${PREFIX}/bin/hrktorrent
