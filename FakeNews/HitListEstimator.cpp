#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

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
    _hitlist(util::load_words(path))
{
    this->article(article);

    if (_hitlist.empty()) log::warning(path) << "Hit list is empty" << endl;

    // Convert the hitlist to upper case, for case-insensitive search.
    for (string& s : _hitlist) s = util::upper(std::move(s));
}

Estimator& HitListEstimator::article(const article::Article* article)
{
    _article = article;
    _upper = util::upper(_article->contents());
    _words = util::count_words(_upper);
    return *this;
}

Estimate HitListEstimator::estimate()
{
    Estimate result = Estimate { 0.0, 0.0 };

    // If the article or hitlist are empty, just return with 0 confidence. This also prevents
    // divide-by-zero errors further down the line.
    if (!_words && _hitlist.empty()) return result;

    string::size_type hits = 0;

    // Finding the same word multiple times increases the number of hits.
    for (const string& s: _hitlist) hits += util::occurrences(_upper, s);

    // The veracity is based on the number of words found per article word. (This number is between
    // 0 and 1.) For example, if no words are found, veracity should be 1. If every word is a match,
    // veracity should be 0. There's a formula for this:

    // (<proportion of words which match> - 1) ^ 10 = veracity.

    // We use a non-linear formula so that having only a few matches drastically reduces the
    // veracity, and the more matches we have, the less it affects things. So 5% matches and 10%
    // matches are very different, but 85% and 90% not so much.

    // The exponent is large because dicking around on a graph plotter, 10% of words matching equals
    // about 35% veracity with an exponent of 10, which seems about right. Less matches greatly
    // increase veracity, and more matches slightly decrease it.

    result.veracity = std::pow(((float)hits / _words) - 1, 10);

    // Next, we need to take the size of the hitlist into account. A huge hitlist is more likely to
    // have matches regardless of the veracity of the article. Therefore a tiny hitlist should bring
    // the veracity down, because if we had loads of matches but a really small hitlist, it seems
    // more fake than an article with the same number of matches with a huge hitlist. (Even though
    // the matches are the same of course.) I have a formula for this, too:

    // 1 - (1 / ((<size of the hitlist> / 100) + 1)) = hitlist multiplier

    // The '100' means that at 100 words, the veracity is halved. This number is just a guess
    // at what seems sensible. As the number of hitlist words tends to infinity, the multiplier
    // converges towards 1 (so no difference to the veracity.) A hitlist of 0 means that the article
    // is always fake. (But the confidence will be 0 anyway in this case.)

    result.confidence = (float)(1 - ((double)1 / (((double)_hitlist.size() / 100) + 1)));
    result.veracity *= result.confidence;

    // "Wait! Why did you set the confidence to this?" -- You
    // It's because the confidence should be based on the size of the hitlist. This formula seems to
    // perfectly describe what I reckon the confidence should be, so screw it.

    // Now we do the same thing again with the size of the article, so tiny articles give less
    // confidence.

    result.confidence *= (float)(1 - ((double)1 / (((double)_words / 100) + 1)));

    return result;
}

}

}
