#include <string>
#include <sstream>
#include <regex>
#include <iostream>

#include <stdlib.h>

#include "curl/curl.h"

#include "net.h"
#include "exc.h"
#include "util.h"

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

// Helper for `Address::operator==()`. Basically, given everything after the first '/' in a URL,
// will strip anything off the end that doesn't make a difference.
string strip_request(const string& s)
{
    string result = s;
    result.erase(std::find(result.cbegin(), result.cend(), '?'));
    result.erase(std::find(result.cbegin(), result.cend(), '#'));
    if (result.back() == '/') result.pop_back();
    result.shrink_to_fit();
    return result;
}

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

bool Address::operator==(const Address& a) const
{
    // TODO If there is a slash on the end, and there is no dot between the slash before that and
    // the end, remove the slash on the end.

    // This seems stupid, and it is, but we're making copies anyway by converting the parts to upper
    // case, so screw it. (This is for case-insensitivity.)
    Address a1 = Address(util::upper(_full));
    Address a2 = Address(util::upper(a.full()));

    // If there is no protocol, assume HTTP. HTTP and HTTPS are the same.
    if
    (
        (a1.protocol().empty() || a1.protocol() == "HTTPS" ? "HTTP" : a1.protocol())
            !=
        (a2.protocol().empty() || a2.protocol() == "HTTPS" ? "HTTP" : a2.protocol())
    )
        return false;

    // If adding 'WWW.' to the beginning of either resource portion of the two addresses would make
    // them the same, they are considered the same.
    if
    (
        a1.resource() != a2.resource()
            &&
        !(
            string("WWW.") + a1.resource() == a2.resource()
                ||
            a1.resource() == string("WWW.") + a2.resource()
        )
    )
        return false;

    // Don't count any junk at the end of the URL.
    return strip_request(a1.request()) == strip_request(a2.request());
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
