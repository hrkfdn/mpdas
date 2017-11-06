#ifndef _CONFIG_H
#define _CONFIG_H

enum ScrobblingService {
	LastFm,
	LibreFm
};

class CConfig
{
public:
	CConfig(char* cfg);

	ScrobblingService getService();

	std::string Get(std::string name);
	bool GetBool(std::string name);
	int GetInt(std::string name);
	void Set(std::string name, std::string value) { _configuration[name] = value; };

	bool gotNecessaryData() {
		if(!Get("username").size() || !Get("password").size())
			return false;
		return true;
	}

	void LoadConfig(std::string path);
private:
	void ParseLine(std::string line);
	std::map<std::string, std::string> _configuration;
};

extern CConfig* Config;

#endif
