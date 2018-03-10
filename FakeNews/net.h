#pragma once

// This file contains anything to do with networking or the Internet.

#include <string>

namespace fakenews
{

namespace net
{

// Represents a web address, split into its constituent parts for easier usage.
class Address
{
    public:
    // Splits the input address into its different parts.
    // address: The address to split.
    // Throws `exc::format` if the string isn't a valid URL.
    Address(const std::string& address);

    const std::string& protocol(); // Can be empty.
    const std::string& resource();
    const std::string& request();  // Can be empty.
    const std::string& full();     // Returns what you passed into the constructor.

    private:
    // In http://www.website.com/folder/page.html#section?value:
    std::string _protocol; // http
    std::string _resource; // www.website.com
    std::string _request;  // folder/page.html#section?value
    std::string _full;     // http://www.website.com/folder/page.html#section?value
};

// Downloads a file from a URL via HTTP. Intended for use with web pages.
// address: The address of the file to download.
// Returns a `string` containing the contents of the file.
// TODO This will probably throw stuff, document that here.
std::string get_file(const std::string& address);

}

}
