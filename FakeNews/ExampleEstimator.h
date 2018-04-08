#pragma once

// This file is just for testing/demonstrating deriving from the `Estimator` class. It just returns
// a veracity/confidence estimate of 1.0.

#include "Estimator.h"

namespace fakenews
{

namespace estimator
{

class ExampleEstimator : public Estimator
{
    public:
    // Example constructor with some extra things we want passed in.
    ExampleEstimator(const article::Article* article, const string& extra_info):
        Estimator(article),
        _extra_info(extra_info)
    { }

    Estimate estimate()
    {
        // Do something based on `_article.heading()`, `_article.contents()` and
        // `_article.address()`. These are data members of the parent class `Estimator`, and have 
        // been set up for us.

        // The heading and contents are `string`s, but please look at "net.h", at the `Address` 
        // type, because `_article.address()` returns this.

        // Let's say that after doing whatever it is we do, we've decided that this article is 100% 
        // legitimate, and we're absolutely certain.

        // `veracity` = 1.0, `confidence` = 1.0
        // Please see "Estimator.h" for more details on `Estimate`.
        return Estimate { 1.0, 1.0 };
    }

    private:
    string _extra_info; // Some extra data member we want to have passed into the constructor.
};

}

}
