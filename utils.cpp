#include "mpdas.h"

#define RED "\x1b[31;01m"
#define RESET "\x1b[0m"
#define GREEN "\x1b[32;01m"
#define YELLOW "\x1b[33;01m"

std::string
md5sum(const char* fmt, ...)
{
	va_list ap;
	char* abuf;
	md5_state_t state;
	unsigned char md5buf[16];

	va_start(ap, fmt);
	vasprintf(&abuf, fmt, ap);
	va_end(ap);

	std::string buf(abuf);
	free(abuf);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t*)buf.c_str(), buf.length());
	md5_finish(&state, md5buf);

	std::ostringstream ret;
	for(unsigned int i = 0; i < 16; i++)
		ret << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)md5buf[i];
	return ret.str();
}

std::string
timestr()
{
	std::stringstream buf;
	time_t rawtime = time(NULL);
	struct tm* t = localtime(&rawtime);

	if(t->tm_hour < 10)
		buf << 0;
	buf << t->tm_hour << ":";
	if(t->tm_min < 10)
		buf << "0";
	buf << t->tm_min << ":";
	if(t->tm_sec < 10)
		buf << "0";
	buf << t->tm_sec;

	return buf.str();
}

void
eprintf(const char* fmt, ...)
{
	char* abuf;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&abuf, fmt, ap);
	va_end(ap);

	std::string buf(abuf);
	free(abuf);

	std::cerr << "(" << timestr() << ") [" << RED << "ERROR" << RESET << "] " << buf << std::endl;
}

void
iprintf(const char* fmt, ...)
{
	time_t rawtime = time(NULL);
	struct tm* t = localtime(&rawtime);
	char* abuf;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&abuf, fmt, ap);
	va_end(ap);

	std::string buf(abuf);
	free(abuf);

	std::cerr << "(" << timestr() << ") [" << YELLOW << "INFO" << RESET << "] " << buf << std::endl;
}

