#include "mpdas.h"

CConfig* Config = 0;

void
CConfig::ParseConfigLine(const std::string& line)
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
		else if(tokens[0] == "debug") {
			if(tokens[1] == "1" || tokens[1] == "true")
				_debug = true;
		}
	}
}

void
CConfig::ParseExcludesLine(const std::string& line)
{
    // could be more robust here
    _excludes.push_back(line);
}

void
CConfig::LoadConfig(const std::string& path)
{
	std::string line = "";

	std::ifstream ifs(path.c_str(), std::ios::in);

	if(!ifs.good()) {
		iprintf("Config file (%s) does not exist or is not readable.", path.c_str());
		return;
	}

	while(ifs.good()) {
		getline(ifs, line);
		ParseConfigLine(line);
	}

}

void
CConfig::LoadExcludes(const std::string& path)
{
    unsigned int numexcls = 0;    
    std::string line = "";
    std::ifstream ifs(path.c_str(), std::ios::in);

	if(!ifs.good()) {
		iprintf("Excludes file (%s) does not exist or is not readable.", path.c_str());
		return;
	}

    while(ifs.good()) {
		getline(ifs, line);
		ParseExcludesLine(line);
        ++numexcls;
	}

    iprintf("Excluding (%i) artists from scrobbling.", numexcls);
}

const bool CConfig::IsArtistExcluded(const std::string& artist) const
{
    std::vector<std::string>::const_iterator it;

    for(it = _excludes.begin(); it != _excludes.end(); ++it)
    {
        if(*it == artist) {
            iprintf("Excluding artist (%s) from scrobble.", artist.c_str());
            return true;
        }
    }

    return false;
}

CConfig::CConfig(const char* cfg, const char* excl)
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

    if(!excl) {
		path = CONFDIR;
		path.append("/mpdasexc");
	}
	else {
		path = excl;
	}

	LoadExcludes(path);
}
