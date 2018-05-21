#pragma once

// This file contains a class which uses black/whitelists of known fake news sites to estimate.

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Estimator.h"
#include "Article.h"
#include "fs.h"
#include "util.h"
#include "net.h"

namespace fakenews
{

namespace estimator
{

class BlackWhiteEstimator : public Estimator
{
    public:
    // BlackWhiteEstimator constructor.
    // article:   The first article to estimate.
    // blacklist: The path to a file of blacklisted sites.
    // whitelist: The path to a file of whitelisted sites.
    // Throws `exc::format` if both the blacklist and whitelist contain an equivalent URL.
    // Throws anything `util::load_clean_warn()` throws.
    // Throws anything `net::Address(const string&)` throws.
    BlackWhiteEstimator(const article::Article* article, const std::string& blacklist_path,
        const std::string& whitelist_path):
        Estimator(article),
        _blacklist(),
        _whitelist()
    {
        vector<string> file_contents = util::load_clean_warn(blacklist_path);
        
        for (string& s : file_contents)
            _blacklist.emplace_back(std::move(util::upper(std::move(s))));
        
        file_contents = util::load_clean_warn(whitelist_path);
        for (string& s : file_contents)
            _whitelist.emplace_back(std::move(util::upper(std::move(s))));

        for (const net::Address& a_w : _whitelist)
        {
            for (const net::Address& a_b : _blacklist)
            {
                if (a_w == a_b)
                {
                    std::stringstream ss;
                    ss << "Whitelist '" << whitelist_path << "' and blacklist '" << blacklist_path
                        << "' contain equivalent URLs '" << a_w.full() << "' and '" << a_b.full()
                        << "' (respectively)";
                    throw exc::format(ss.str());
                }
            }
        }
    }

    Estimate estimate()
    {
        // If we find the address in the whitelist, it's definitely legit.
        if
        (
            std::find(_whitelist.begin(), _whitelist.end(),
                util::upper(_article->address().resource()))
            != _whitelist.end()
        )
        return Estimate { 1.0, 1.0 };

        // If we find it in the blacklist, it's definitely not.
        else if
        (
            std::find(_blacklist.begin(), _blacklist.end(),
                util::upper(_article->address().resource()))
            != _blacklist.end()
        )
        return Estimate { 0.0, 1.0 };

        else return Estimate { 0.0, 0.0 }; // Otherwise, don't know Jeff.
    }

    private:
    std::vector<net::Address> _blacklist;
    std::vector<net::Address> _whitelist;
};

}

}
