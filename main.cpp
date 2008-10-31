#include "mpdas.h"

void
onclose()
{
	iprintf("%s", "Closing mpdas.");

	if(MPD) delete MPD;
	if(AudioScrobbler) delete AudioScrobbler;
	if(Cache) delete Cache;
	if(Config) delete Config;
}

int
main(int argc, char* argv[])
{
	atexit(onclose);

	Config = new CConfig();

	MPD = new CMPD();
	if(!MPD->isConnected())
		return EXIT_FAILURE;

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
