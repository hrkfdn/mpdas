#ifndef _CONFIG_H
#define _CONFIG_H

class CConfig
{
	public:
		CConfig(char* cfg);

		std::string getLUsername() { return _lusername; }
		std::string getLPassword() { return _lpassword; }
		std::string getMHost() { return _mhost; }
		std::string getMPassword() { return _mpassword; }
		std::string getRUser() { return _runninguser; }
		int getMPort() { return _mport; }

		bool gotNecessaryData() { 
			if(!_lusername.size() || !_lpassword.size())
				return false;
			return true;
		}
		void LoadConfig(std::string path);
	private:
		void ParseLine(std::string line);
		std::string _lusername, _lpassword;
		std::string _mhost, _mpassword;
		std::string _runninguser;
		int _mport;
};

extern CConfig* Config;

#endif
