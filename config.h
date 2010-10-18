#ifndef _CONFIG_H
#define _CONFIG_H

class CConfig
{
	public:
		CConfig(char* cfg, char* excl);

		std::string getLUsername() { return _lusername; }
		std::string getLPassword() { return _lpassword; }
		std::string getMHost() { return _mhost; }
		std::string getMPassword() { return _mpassword; }
		std::string getRUser() { return _runninguser; }
		bool getDebug() { return (_debug == true); }
		int getMPort() { return _mport; }

		bool gotNecessaryData() { 
			if(!_lusername.size() || !_lpassword.size())
				return false;
			return true;
		}
		void LoadConfig(std::string path);
        void LoadExcludes(std::string path);
        bool IsArtistExcluded(std::string artist);
	private:
		void ParseConfigLine(std::string line);
        void ParseExcludesLine(std::string line);
		std::string _lusername, _lpassword;
		std::string _mhost, _mpassword;
		std::string _runninguser;
        std::vector<std::string> _excludes;
		int _mport;
		bool _debug;
};

extern CConfig* Config;

#endif
