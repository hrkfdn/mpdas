#ifndef _CONFIG_H
#define _CONFIG_H

class CConfig
{
	public:
		CConfig(const char* cfg, const char* excl);

		const std::string getLUsername() const { return _lusername; }
		const std::string getLPassword() const { return _lpassword; }
		const std::string getMHost() const { return _mhost; }
		const std::string getMPassword() const { return _mpassword; }
		const std::string getRUser() const { return _runninguser; }
		const bool getDebug() const { return (_debug == true); }
		const int getMPort() const { return _mport; }

		const bool gotNecessaryData() const { 
			if(!_lusername.size() || !_lpassword.size())
				return false;
			return true;
		}
		void LoadConfig(const std::string& path);
        void LoadExcludes(const std::string& path);
        const bool IsArtistExcluded(const std::string& artist) const;
	private:
		void ParseConfigLine(const std::string& line);
        void ParseExcludesLine(const std::string& line);
		std::string _lusername, _lpassword;
		std::string _mhost, _mpassword;
		std::string _runninguser;
        std::vector<std::string> _excludes;
		int _mport;
		bool _debug;
};

extern CConfig* Config;

#endif
