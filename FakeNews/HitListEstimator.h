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
    // Throws `exc::file` if the file at `path` couldn't be opened for reading.
    HitListEstimator(const article::Article* article, const std::string& path);
    Estimate estimate();

    private:
    std::string _upper; // Uppercase copy of the article to make things case-insensitive, ha.
    std::vector<std::string> _hitlist;
};

}

}
