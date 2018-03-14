#pragma once

// This file contains the definition for an `Article`, which is a news item.

#include <string>

#include "net.h"

namespace fakenews
{

namespace article
{

class Article
{
    public:
    Article(const std::string& headline, const std::string& contents, const net::Address& address):
        _headline(headline),
        _contents(contents),
        _address(address)
    { }

    Article(const std::string& HTML); // TODO implement this, get the headline and contents from
                                      // HTML

    const std::string& headline() const { return _headline; }
    const std::string& contents() const { return _contents; }
    const net::Address& address() const { return _address;  }

    protected:
    std::string  _headline;
    std::string  _contents;
    net::Address _address;
};

}

}