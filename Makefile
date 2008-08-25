CXX	?= g++
OBJ	= main.o md5.o utils.o mpd.o audioscrobbler.o cache.o
OUT	= mpdas

CFLAGS 	= `pkg-config --cflags libmpd libcurl` -s -O2 -pipe -g
LDFLAGS	= `pkg-config --libs libmpd libcurl` -g

all: $(OUT)

%.o: %.cpp
	$(CXX) -c -o $@ $<

$(OUT): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(OUT)

clean:
	rm -rf $(OBJ) $(OUT)

