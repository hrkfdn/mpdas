#include "mpdas.h"

int IniHandler(void* param, const char* section, const char* name, const char* value)
{
    CConfig* config = (CConfig*)param;
    std::string val = std::string(value);

    // strip quotes if they exist to allow passwords to begin with a whitespace
    if(val.length() >= 2 && val[0] == '\"' && val[val.length()-1] == '\"') {
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
    std::string host = "localhost";
	std::string port = "6600";

    /* Set optional settings to default */
    if(const char* env_host = std::getenv("MPD_HOST"))
    	host = env_host;

	if (const char* env_port = std::getenv("MPD_PORT") )
		port = env_port;

    Set("host", host);
    Set("port", port);
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
