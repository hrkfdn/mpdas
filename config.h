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
		std::string getUPnPName() { return _upnpname; }
		bool getDebug() { return (_debug == true); }
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
		std::string _upnpname;
		int _mport;
		bool _debug;
};

extern CConfig* Config;

#endif
