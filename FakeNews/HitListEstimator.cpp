#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "HitListEstimator.h"

#include "log.h"
#include "fs.h"
#include "exc.h"

using std::endl;
using std::string;
using std::vector;

namespace fakenews
{

namespace estimator
{

HitListEstimator::HitListEstimator(const article::Article* article, const string& path):
    Estimator(article),
    _upper(_article->contents()),
    _hitlist()
{
    std::ifstream file(path);

    if (!file)
    {
        char err[256];
        std::stringstream ss;
        ss << "Could not open file '" << path << "' for reading: " << fs::error(err, sizeof(err));
        throw exc::file(ss.str());
    }

    // Load each line into a vector, converting to upper case as we go.
    string line;
    string allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZ- "; // The characters allowed in hit list entries.

    for (decltype(_hitlist.size()) i = 1; std::getline(file, line); ++i)
    {
        for (string::size_type j = 0; j < line.size(); ++j)
        {
            line[j] = toupper(line[j]);

            if (allowed.find(line[j]) == string::npos)
            {
                // TODO Have `exc::format` optionally take a file, line and column in its
                // constructor, give those to it here, and pass them to `log::error` wherever this
                // exception is caught.

                std::stringstream ss;

                ss << "Invalid character '" << line[j] << "' in hit list entry '" << line
                    << "', only '" << allowed << "' are allowed";

                throw exc::format(ss.str());
            }
        }

        if (std::find(_hitlist.cbegin(), _hitlist.cend(), line) != _hitlist.cend())
            log::warning(path, i) << "Duplicate entry '" << line << "' in hit list";

        _hitlist.emplace_back(line);
    }

    if (_hitlist.empty()) log::warning(path) << "Hit list is empty";

    // Make the article copy upper case.
    for (char& c : _upper) c = toupper(c);
}

Estimate HitListEstimator::estimate()
{
    Estimate result = Estimate { 0.0, 0.0 };

    size_t hits = 0;

    for (const string& s : _hitlist) { if (_upper.find(s) != string::npos) { ++hits; continue; } }

    log::log << "Hit list word matches: " << hits << endl;

    // TODO The veracity estimate is ABSOLUTELY HORRIBLE, and should be replaced by something better
    // ASAP.

    // It's the length of the article in bytes, divided by 5, then divided by the number of hits,
    // and then (1 / that) to make it between 0 and 1. Then we invert it (subtract it from 1) 
    // because we want a SMALLER number the more hits we have, not a larger one. By tweaking the '5' 
    // higher, we can make each hit hurt the veracity more.

    // Basically we have the number of hits. We need to find a way to translate this into a
    // 'fakeness' rating from 0 to 1. We should probably count the words in the article and base it
    // on the proportion of words which are hits to words which aren't. We should also take the 
    // length of our hitlist into consideration somehow, because a short hitlist is unreliable.
    //
    // Hell, a long hitlist is pretty unreliable, but oh well.

    // +1 so that if we have no hits, we get 0. (I think.)
    result.veracity = 1 - (1.0f / ((_upper.size() / 5.0f) / (hits + 1)));

    // This might not actually be that crappy. It's just 1 divided by the number of entries in the
    // hit list. Then we subtract that from 1. As we get more and more hitlist entries, the number
    // converges towards 1.

    // +1 so that if the hitlist is empty, we get 0.
    result.confidence = 1 - (1.0f / (_hitlist.size() + 1));

    return result;
}

}

}