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
    // Throws anything `fs::load_lines()` throws.
    // Throws anything `net::Address(string)` throws.
    Article(const std::string& path);
    
    Article(const Article& a):
        _headline(a.headline()),
        _contents(a.contents()),
        _address(new net::Address(a.address()))
    { }

    ~Article() { delete _address; }   

    const std::string& headline() const { return _headline; }
    const std::string& contents() const { return _contents; }
    const net::Address& address() const { return *_address; }

    protected:
    std::string _headline;
    std::string _contents;
    
    // This is a pointer because `net::Address` has no default constructor, and the default
    // constructor is implicitly called on the members by any constructors we create for `Article`
    // with MSVC (for some bizarre reason) so instead of actually fixing the problem, screw it,
    // we'll just make it a pointer.
    net::Address* _address;
};

}

}