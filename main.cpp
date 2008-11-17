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

void
setid(char* username)
{
	passwd* userinfo = 0;

	if(getuid != 0) {
		eprintf("%s", "You are not root.");
		exit(EXIT_FAILURE);
	}

	userinfo = getpwnam(username);
	if(!userinfo) {
		eprintf("%s", "The user you specified does not exist.");
		exit(EXIT_FAILURE);
	}

	if(setuid(userinfo->pw_uid == -1 || setgid(userinfo->pw_gid))) {
			eprintf("%s %s", "Could not switch to user", username);
			exit(EXIT_FAILURE);
	}

	setenv("HOME", userinfo->pw_dir, 0);
}

int
main(int argc, char* argv[])
{
	atexit(onclose);

	if(argc >= 2)
		setid(argv[2]);

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
