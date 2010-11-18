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
setid(const char* username)
{
	passwd* userinfo = 0;

	if(strlen(username) == 0)
		goto homecfg;

	if(getuid() != 0) {
		eprintf("%s", "You are not root. Not changing user ..");
		return;
	}

	userinfo = getpwnam(username);
	if(!userinfo) {
		eprintf("%s", "The user you specified does not exist.");
		exit(EXIT_FAILURE);
	}

	if(setgid(userinfo->pw_gid == -1 || setuid(userinfo->pw_uid)) == -1) {
			eprintf("%s %s", "Could not switch to user", username);
			exit(EXIT_FAILURE);
	}

	setenv("HOME", userinfo->pw_dir, 1);

homecfg:
	// Load config in home dir as well (if possible)
	std::string path = getenv("HOME");
	path.append("/.mpdasrc");
	Config->LoadConfig(path);
}

void
printversion()
{
	fprintf(stdout, "mpdas-"VERSION", (C) 2010 Henrik Friedrichsen.\n");
	fprintf(stdout, "Global config path is set to \"%s\"\n", CONFDIR);
}

void
printhelp()
{
	fprintf(stderr, "\nusage: mpdas [-h] [-v] [-c config]\n");

	fprintf(stderr, "\n\th: print this help");
	fprintf(stderr, "\n\tv: print program version");
	fprintf(stderr, "\n\tc: load specified config file");

	fprintf(stderr, "\n");
}

int
main(int argc, char* argv[])
{
	int i;
	char* config = 0;
	bool go_daemon = false;

	if(argc >= 2) {
		for(i = 1; i <=  argc-1; i++) {
			if(strstr(argv[i], "-h") == argv[i]) {
				printversion();
				printhelp();
				return EXIT_SUCCESS;
			}
			if(strstr(argv[i], "-v") == argv[i]) {
				printversion();
				return EXIT_SUCCESS;
			}

			else if(strstr(argv[i], "-c") == argv[i]) {
				if(i >= argc-1) {
					fprintf(stderr, "mpdas: config path missing!\n");
					printhelp();
					return EXIT_FAILURE;
				}
				config = argv[i+1];
			}

			else if(strstr(argv[i], "-d") == argv[i]) {
				go_daemon = true;
			}
		}
	}

	atexit(onclose);

	Config = new CConfig(config);

	setid(Config->getRUser().c_str());

	if(!Config->gotNecessaryData()) {
		eprintf("%s", "AudioScrobbler username or password not set.");
		return EXIT_FAILURE;
	}

	if (go_daemon) {
		if (daemon(1, 0)) {
			perror("daemon");
			return EXIT_FAILURE;
		}
	}

	MPD = new CMPD();
	if(!MPD->isConnected())
		return EXIT_FAILURE;

	AudioScrobbler = new CAudioScrobbler();
	AudioScrobbler->Handshake();
	Cache = new CCache();
	Cache->LoadCache();

scan:
	MPD->Update();
	AudioScrobbler->GetLove();
	Cache->WorkCache();
	usleep(500000);
	goto scan;

	return EXIT_SUCCESS;
}
