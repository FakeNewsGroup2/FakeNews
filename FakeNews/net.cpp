#include <string>
#include <sstream>
#include <regex>
#include <iostream>

#include <stdlib.h>

#include "curl/curl.h"

#include "net.h"
#include "exc.h"

using std::string;

namespace fakenews
{

namespace net
{

// Helpers for `get_file()`.
// Used for storing the HTML response.
struct sstring { char* data; size_t size; };
// Write callback function when receiving data from libcurl. Purely part of `get_file()`.
size_t write_callback(char* p, size_t size, size_t nmemb, void* data);

Address::Address(const string& address):
    _protocol(),
    _resource(),
    _request(),
    _full(address)
{
    // TODO This regex isn't perfect. It doesn't handle IPv6 addresses, usernames and passwords,
    // port numbers, and other (presumably unlikely) stuff.
    // See https://en.wikipedia.org/wiki/URL#Syntax.
    static std::regex r
    (R"end((([a-zA-Z0-9]+)://)?([A-Za-z0-9_\.\-]+(\.[A-Za-z0-9_\.\-]+)+)(/(.*))?)end");

    std::smatch m;

    if (!std::regex_search(address, m, r)) throw exc::format("Invalid URL", address);

    _protocol = m[2];
    _resource = m[3];
    _request  = m[6];
}

std::string get_file(const std::string& address)
{
    CURL* handle = curl_easy_init();

    if (!handle) throw exc::net("Failed to initialise session", "libcurl");

    struct sstring response;
    response.data = NULL;
    response.size = 0;

    curl_easy_setopt(handle, CURLOPT_URL, address.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
    
    // Yeah, I know this is insecure. But it's sure as hell easier than finding out how to get SSL
    // certificates on Windows...
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);

    // TODO perhaps we could use a user agent here?
    // curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        // "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.181 Safari/537.36");

    CURLcode status = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    if (status != CURLE_OK) throw exc::net(curl_easy_strerror(status), address);

    string result(response.data);
    free(response.data);

    return result;
}

// TODO Try and use more C++ containers for all of this curl jazz.

// Write callback function when receiving data from libcurl. Purely part of `get_file()`.
size_t write_callback(char* p, size_t size, size_t nmemb, void* data)
{
    sstring* mem = (sstring*)data;
    size_t new_size = size * nmemb;

    mem->data = (char*)realloc(mem->data, mem->size + new_size + 1);
    memcpy(mem->data + mem->size, p, new_size);
    mem->size += new_size;
    mem->data[mem->size] = '\0';
    return size * nmemb;
}

}

}
