#pragma once

// This file contains a class which uses black/whitelists of known fake news sites to estimate.

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "Estimator.h"
#include "Article.h"
#include "fs.h"

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
    // Throws anything `fs::load_lines()` throws.
    BlackWhiteEstimator(const article::Article* article, const std::string& blacklist,
        const std::string& whitelist):
        Estimator(article),
        _blacklist(fs::load_lines(blacklist)),
        _whitelist(fs::load_lines(whitelist))
    {
        // TODO remove blank entries from the black/whitelist
        // TODO if there are any sites present in both lists, throw something
    }

    Estimate estimate()
    {
        // If we find the address in the whitelist, it's definitely legit.
        if
        (
            std::find(_whitelist.begin(), _whitelist.end(), _article->address().resource())
            != _whitelist.end()
        )
        return Estimate { 1.0, 1.0 };

        // If we find it in the blacklist, it's definitely not.
        else if
        (
            std::find(_blacklist.begin(), _blacklist.end(), _article->address().resource())
            != _blacklist.end()
        )
        return Estimate { 0.0, 1.0 };

        else return Estimate { 0.0, 0.0 }; // Otherwise, don't know Jeff.
    }

    private:
    std::vector<std::string> _whitelist;
    std::vector<std::string> _blacklist;
};

}

}