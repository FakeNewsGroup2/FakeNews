#pragma once

// This file contains the definition for the `Estimator` class, which is an abstract base class
// intended to be derived from.

#include "Article.h"

namespace fakenews
{

namespace estimator
{

// The results of an estimate.
struct Estimate
{
    // These variables will apply to some methods of estimating more than others.
    float veracity;   // How legitimate the article seems, from 0 to 1.
    float confidence; // How sure we are, from 0 to 1. (0 means that we couldn't tell, or our method
                      // wasn't applicable, etc.)
};

class Estimator
{
    public:
    Estimator(const article::Article* article): _article(article) { }
    Estimator(const Estimator& e): _article(e._article) { }
    Estimator& operator= (const Estimator& e) { _article = e._article; return *this; }

    virtual ~Estimator() { }

    Estimator& article(const article::Article* article)
    {
        _article = article;
        return *this;
    }

    // You're meant to override this.
    virtual Estimate estimate() { return Estimate { 0.0, 0.0 }; }

    protected:
    const article::Article* _article;
};

}

}
