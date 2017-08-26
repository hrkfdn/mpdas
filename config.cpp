#include "mpdas.h"

CConfig* Config = 0;

int IniHandler(void* param, const char* section, const char* name, const char* value)
{
    CConfig* config = (CConfig*)param;
    std::string val = std::string(value);

    // strip quotes if they exist to allow passwords to begin with a whitespace
    if(val.length() >= 2 && val.front() == '\"' && val.back() == '\"') {
	val.erase(0, 1);
	val.erase(val.length() - 1);
    }

    config->Set(name, val);

    return 1;
}

void CConfig::LoadConfig(std::string path)
{
    if(ini_parse(path.c_str(), &IniHandler, this) < 0) {
	iprintf("Cannot parse config file (%s).", path.c_str());
	return;
    }
}
std::string CConfig::Get(std::string name)
{
    if(_configuration.find(name) == _configuration.end()) {
	return "";
    }

    return _configuration.find(name)->second;
}

bool CConfig::GetBool(std::string name)
{
    std::string value = Get(name);
    return value == "1" || value == "true";
}

int CConfig::GetInt(std::string name)
{
    return atoi(Get(name).c_str());
}

ScrobblingService CConfig::getService()
{
    return Get("service") == "librefm" ? LibreFm : LastFm;
}

CConfig::CConfig(char* cfg)
{
    /* Set optional settings to default */
    Set("host", "localhost");
    Set("port", "6600");
    Set("debug", "false");
    Set("service", "lastfm");

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
