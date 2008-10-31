#ifndef _CONFIG_H
#define _CONFIG_H

class CConfig
{
	public:
		CConfig();
	private:
		void ParseLine(std::string line);
};

extern CConfig* Config;

#endif
