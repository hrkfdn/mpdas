#include "mpdas.h"
#include <regex>

CConfig* Config = 0;

std::vector<std::string> split_first(const std::string line, const std::regex re)
{
	std::vector<std::string> result{
		std::sregex_token_iterator{line.begin(), line.end(), re, {-1, 0}}, {}
	};

	if (result.size() > 3) {
		std::string first = result[0];
		std::string rest;

		// merge rest of the splitted parts into one string
		for (std::vector<std::string>::const_iterator i = result.begin() + 2;
				i != result.end();
				++i) {
			rest += *i;
		}

		return {first, rest};
	} else if (result.size() == 3) {
		return {result[0], result[2]};
	} else if (result.size()) {
		return {result[0]};
	} else {
		return {};
	}
}

void CConfig::ParseLine(std::string line)
{
	std::vector<std::string> tokens;

	std::regex delimiter_re("([=|:]|[[:blank:]])+");

	if(line.size() > 1) {
		tokens = split_first(line, delimiter_re);
	}

	if(tokens.size() > 1) {
		if(tokens[0] == "username")
			_lusername = tokens[1];
		else if(tokens[0] == "password")
			_lpassword = tokens[1];
		else if(tokens[0] == "host")
			_mhost = tokens[1];
		else if(tokens[0] == "mpdpassword")
			_mpassword = tokens[1];
		else if(tokens[0] == "service" && tokens[1] == "librefm")
			_service = LibreFm;
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

void CConfig::LoadConfig(std::string path)
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
	_service = LastFm;
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
