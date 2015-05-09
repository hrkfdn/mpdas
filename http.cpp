#include "http.hpp"

#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <curl/curl.h>
#include <iostream>

// curl_global_init() and curl_global_cleanup() ??
Http::Http()
{
    _handle = curl_easy_init();
    if(!_handle)
        throw std::runtime_error("curl_easy_init() failed");

    curl_easy_setopt(_handle, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, readResponse);
}

Http::~Http()
{
    curl_easy_cleanup(_handle);
}

int Http::post(std::string url, std::string body)
{
    int ret;
    std::ostringstream buf;

    curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(_handle, CURLOPT_POST, 1);
    curl_easy_setopt(_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, body.c_str());

    ret = curl_easy_perform(_handle);
    if(!ret)
        _response = buf.str();
    else
        _response.clear();

    return ret;
}

int Http::post(std::string url, std::map<std::string, std::string> params)
{
    std::ostringstream body;

    // Create a message in application/x-www-form-urlencoded format
    for(auto param : params)
    {
        char* escaped = curl_easy_escape(_handle, param.second.c_str(), 0);
        body << "&" << param.first << "=" << std::string(escaped);
        curl_free(escaped);
    }

    return post(url, body.str());
}

// TODO: Check if this works when query is empty
std::string Http::urlencode(std::string query) const
{
    std::string ret;

    char* buf = curl_easy_escape(_handle, query.c_str(), 0);
    ret = std::string(buf);
    curl_free(buf);

    return ret;
}

size_t Http::readResponse(char* ptr, size_t size, size_t nmemb, void* osstream)
{
    static_cast<std::ostringstream*>(osstream)->write(ptr, size*nmemb);
    return size*nmemb;
}

