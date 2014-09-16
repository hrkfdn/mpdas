#include "mpdas.h"
#include <wordexp.h>

CConfig* Config = 0;

std::string 
CConfig::ExpandEnvironment(std::string value) 
{
	wordexp_t result;
	std::string ret = "";
	if(wordexp(value.c_str(), &result, WRDE_NOCMD)) {
		wordfree(&result);
		return value;
	}
	for(int i = 0; i < result.we_wordc; i++) {
		ret.append(result.we_wordv[i]);
	}
	wordfree(&result);
	return ret;
}

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
			_lusername = ExpandEnvironment(tokens[1]);
		else if(tokens[0] == "password")
			_lpassword = ExpandEnvironment(tokens[1]);
		else if(tokens[0] == "host")
			_mhost = ExpandEnvironment(tokens[1]);
		else if(tokens[0] == "mpdpassword")
			_mpassword = ExpandEnvironment(tokens[1]);
		else if(tokens[0] == "port")
			_mport = atoi(tokens[1].c_str());
		else if(tokens[0] == "runas")
			_runninguser = tokens[1];
		else if(tokens[0] == "debug") {
			if(tokens[1] == "1" || tokens[1] == "true")
				_debug = true;
		}

	}
}

void
CConfig::LoadConfig(std::string path)
{
	std::string line = "";

	std::ifstream ifs(path.c_str(), std::ios::in);

	if(!ifs.good()) {
		iprintf("Config file (%s) does not exist or is not readable.", path.c_str());
		return;
	}

	while(ifs.good()) {
		getline(ifs, line);
		ParseLine(line);
	}

}

CConfig::CConfig(char* cfg)
{
	/* Set optional settings to default */
	_mhost = "localhost";
	_mport = 6600;
	_debug = false;

	std::string path = "";

	if(!cfg) {
		path = CONFDIR;
		path.append("/mpdasrc");
	}
	else {
		path = cfg;
	}

	LoadConfig(path);
}
