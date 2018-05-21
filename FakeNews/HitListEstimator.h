#pragma once

// This file contains the definition for the `HitListEstimator` class, which guesses fakeness by 
// counting occurrences of blacklisted words.

#include <string>
#include <vector>

#include "Estimator.h"

namespace fakenews
{

namespace estimator
{

class HitListEstimator : public Estimator
{
    public:
    // Estimates based on the presence of words from a 'hit list,' which is a file where each line
    // contains a 'bad' word to look for.
    // article: The article to estimate.
    // path:    The path to the hit list.
    // Throws anything `util::load_words()` throws.
    HitListEstimator(const article::Article* article, const std::string& path);
    Estimate estimate();

    Estimator& HitListEstimator::article(const article::Article* article);

    private:
    std::string _upper; // Uppercase copy of the article to make things case-insensitive, ha.
    std::string::size_type _words; // The number of words in `_upper`, pre-computed for efficiency.
    std::vector<std::string> _hitlist;
};

}

}
