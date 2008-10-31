#include "mpdas.h"

CConfig* Config = 0;

void
CConfig::ParseLine(std::string line)
{

}

CConfig::CConfig()
{
	std::string line = "";
	std::string path = getenv("HOME");
	path.append("/.mpdasrc");
	std::ifstream ifs(path.c_str(), std::ios::in);

	while(ifs.good()) {
		ifs >> line;
		ParseLine(line);
	}
}
