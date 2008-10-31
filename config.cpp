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

	for(int i = 0; i < tokens.size(); i++) {
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
	}
}

CConfig::CConfig()
{
	/* Set optional settings to default */
	_mhost = "localhost";
	_mport = 6600;

	std::string line = "";
	std::string path = getenv("HOME");
	path.append("/.mpdasrc");
	std::ifstream ifs(path.c_str(), std::ios::in);

	if(!ifs.good()) {
		eprintf("%s", "Config file does not exist or is not readable.");
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
