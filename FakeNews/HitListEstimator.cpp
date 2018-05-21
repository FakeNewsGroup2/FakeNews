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
#include "util.h"

using std::endl;
using std::string;
using std::vector;

namespace fakenews
{

namespace estimator
{

HitListEstimator::HitListEstimator(const article::Article* article, const string& path):
    Estimator(article),
    _upper(util::upper(_article->contents())),
    _hitlist(util::load_clean_warn(path, "words"))
{
    if (_hitlist.empty()) log::warning(path) << "Hit list is empty" << endl;

    // TODO Make sure that the hitlist doesn't contain weird characters. There's already code for
    // this in the neural network... move it somewhere else and call it twice.

    // Convert the hitlist to upper case, for case-insensitive search.
    for (string& s : _hitlist) s = util::upper(std::move(s));
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
