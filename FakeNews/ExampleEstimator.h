#pragma once

// This file is just for testing/demonstrating deriving from the `Estimator` class. It just returns
// a veracity/confidence estimate of 1.0.

#include "Estimator.h"

namespace fakenews
{

namespace estimator
{

// The `Estimator` class provides a variable named `_article` of the `Article` type. Right now, this
// just contains three methods `heading()`, `contents()` and `address()` to get info about the
// article. Look at 'net.h' at the `Address` type, because `address()` returns this, not a string.
class ExampleEstimator : public Estimator
{
    public:
    Estimate estimate()
    {
        // Do something based on `_article.heading()`, `_article.contents()` and
        // `_article.address()`.
        return Estimate { 1.0, 1.0 }; // `veracity` = 1.0, `confidence` = 1.0
    }
};

}

}