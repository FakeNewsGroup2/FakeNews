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

Address::Address(const string& address): _full(address)
{
    // TODO This regex isn't perfect. It doesn't handle IPv6 addresses, usernames and passwords,
    // port numbers, and other (presumably unlikely) stuff.
    // See https://en.wikipedia.org/wiki/URL#Syntax.
    static std::regex r
    (R"end((([a-zA-Z0-9]+)://)?([A-Za-z0-9_\.\-]+(\.[A-Za-z0-9_\.\-]+)+)(/(.*))?)end");

    std::smatch m;

    if (!std::regex_search(address, m, r))
        throw exc::format(string("Invalid URL '") + address + '\'');

    _protocol = m[2];
    _resource = m[3];
    _request  = m[6];
}

const string& Address::protocol() { return _protocol; }
const string& Address::resource() { return _resource; }
const string& Address::request()  { return _request;  }
const string& Address::full()     { return _full;     }

std::string get_file(const std::string& address)
{
    CURL* handle = curl_easy_init();

    if (!handle) throw exc::net("Could not initialise libcurl");

    struct sstring response;
    response.data = NULL;
    response.size = 0;

    curl_easy_setopt(handle, CURLOPT_URL, address.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);

    // TODO perhaps we could use a user agent here?
    // curl_easy_setopt(handle, CURLOPT_USERAGENT, "some_user_agent");

    CURLcode status = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    string result = response.data;
    free(response.data);

    if (status != CURLE_OK)
    {
        std::stringstream ss;
        ss << "Could not get file at '" << address << "': " << curl_easy_strerror(status);
        throw exc::net(ss.str());
    }

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