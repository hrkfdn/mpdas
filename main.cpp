#include "mpdas.h"

void
onclose()
{
	iprintf("%s", "Closing mpdas.");
	delete MPD;
	delete AudioScrobbler;
	delete Cache;
}

int
main(int argc, char* argv[])
{
	atexit(onclose);

	MPD = new CMPD();
	if(!MPD->isConnected()) {
		delete MPD;
		return EXIT_FAILURE;
	}

	AudioScrobbler = new CAudioScrobbler();
	AudioScrobbler->Handshake();
	Cache = new CCache();
	Cache->LoadCache();

scan:
	MPD->Update();
	Cache->WorkCache();
	usleep(500000);
	goto scan;

	return EXIT_SUCCESS;
}
