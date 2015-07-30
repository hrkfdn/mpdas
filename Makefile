VERSION = 0.4.3

CXX	?= g++
OBJ	= main.o md5.o utils.o mpd.o audioscrobbler.o cache.o config.o
MAIN_OBJ = main.o
TESTS_DIR = tests
TESTS_OBJ = tests.o
OUT	= mpdas
TESTS_OUT = mpdas_tests
PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/man/man1
CONFIG ?= $(PREFIX)/etc

CXXFLAGS	+= `pkg-config --cflags libmpdclient libcurl`
LIBS		= `pkg-config --libs libmpdclient libcurl`

CXXFLAGS	+= -DCONFDIR="\"$(CONFIG)\"" -DVERSION="\"$(VERSION)\""


all: $(OUT)

.cpp.o:
	@echo [CXX] $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT): $(OBJ)
	@echo [LD] $@
	@$(CXX) $(LDFLAGS) $(OBJ) $(LIBS) -o $(OUT)

$(TESTS_OUT): $(OBJ) $(TESTS_DIR)/$(TESTS_OBJ)
	@echo [LD] $@
	@$(CXX) $(LDFLAGS) $(filter-out $(MAIN_OBJ), $(OBJ)) $(TESTS_DIR)/$(TESTS_OBJ) $(LIBS) -o $(TESTS_DIR)/$(TESTS_OUT)

clean:
	rm -rf $(OBJ) $(OUT) $(TESTS_DIR)/$(TESTS_OBJ) $(TESTS_DIR)/$(TESTS_OUT)

install: all
	install -d ${DESTDIR}${PREFIX}/bin
	install -d ${DESTDIR}${MANPREFIX}
	install -m 755 mpdas ${DESTDIR}${PREFIX}/bin
	install -m 644 mpdas.1 ${DESTDIR}${MANPREFIX}/mpdas.1

uninstall:
	-rm ${DESTDIR}${PREFIX}/bin/mpdas
	-rm ${DESTDIR}${MANPREFIX}/mpdas.1
