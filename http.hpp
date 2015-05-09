#ifndef MPDAS_HTTP_HPP
#define MPDAS_HTTP_HPP

#include <map>
#include <string>
#include <curl/curl.h>

class Http
{
    public:
        Http();
        ~Http();

        int post(std::string url, std::string body);
        int post(std::string url, std::map<std::string, std::string> params);

        std::string urlencode(std::string query) const;

        const std::string& response() const { return _response; };

    private:
        std::string _response;

        CURL* _handle;
        static size_t readResponse(char* ptr, size_t size, size_t nmemb, void* osstream);
};

#endif
