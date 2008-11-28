#include "mpdas.h"

CConfig* Config = 0;

void
CConfig::ParseLine(std::string line)
{
	std::vector<std::string> tokens;
	char* pstr = 0;
	char* szline = new char[line.size()+1];

	strncpy(szline, line.c_str(), line.size()+1);

	pstr = strtok(szline, " :=\t");
	while(pstr) {
		tokens.push_back(pstr);
		pstr = strtok(NULL, " :=\t");
	}
	delete szline;

	if(tokens.size() > 1) {
		if(tokens[0] == "username")
			_lusername = tokens[1];
		else if(tokens[0] == "password")
			_lpassword = tokens[1];
		else if(tokens[0] == "host")
			_mhost = tokens[1];
		else if(tokens[0] == "mpdpassword")
			_mpassword = tokens[1];
		else if(tokens[0] == "port")
			_mport = atoi(tokens[1].c_str());
		else if(tokens[0] == "runas")
			_runninguser = tokens[1];

	}
}

void
CConfig::LoadConfig(std::string path)
{
	std::string line = "";

	std::ifstream ifs(path.c_str(), std::ios::in);

	if(!ifs.good()) {
		eprintf("Config file (%s) does not exist or is not readable.", path.c_str());
		exit(EXIT_FAILURE);
	}

	while(ifs.good()) {
		getline(ifs, line);
		ParseLine(line);
	}

	if(!_lusername.size() || !_lpassword.size()) {
		eprintf("%s", "AudioScrobbler username or password not set.");
		exit(EXIT_FAILURE);
	}

}

CConfig::CConfig(char* cfg)
{
	/* Set optional settings to default */
	_mhost = "localhost";
	_mport = 6600;

	std::string path = "";

	if(!cfg) {
		path = CONFDIR;
		path.append("/mpdasrc");
		if(!fileexists(path.c_str())) {
			iprintf("Global config (%s) does not exist. Falling back to home directory.", path.c_str());
			path = getenv("HOME");
			path.append("/.mpdasrc");
		}
	}
	else {
		path = cfg;
	}

	LoadConfig(path);
}
