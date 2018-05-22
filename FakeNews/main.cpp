#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <memory>
#include <algorithm>
#include <random>

#include <cstdlib>

#include "curl/curl.h"
//include "curl/types.h"
#include "curl/easy.h"
#include <cstdio>

#include "util.h"
#include "log.h"
#include "fs.h"
#include "exc.h"
#include "net.h"
#include "Article.h"
#include "BlackWhiteEstimator.h"
#include "NeuralNet.h"
#include "HitListEstimator.h"
#include "NeuralNetEstimator.h"

#include <map>
#include <cassert>
#include <cmath>
#include <sstream>
#include <regex>

#include <stdio.h>
// CMP2089M-1718 - Group Project
// Fake News Detector
// This program takes a series of URLs to online news articles, and provides an estimate of its
// veracity.

// Group 2
// 16609509 - James Coe
// 16606590 - Paulius Vaitaitis
// 16606229 - Lee Milner
// 15595025 - Ethan Ansell
// 16641828 - Jiahe Wang
// 15625064 - Dahai Zhu
// 16609305 - Ashley Worth

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;

using namespace fakenews;

// Just for testing.
void pause()
{
    cerr << "Press Enter to continue..." << endl;
    std::cin.clear();
    std::cin.get();
}

// We have a class to wrap the whole program, for two reasons:
// We can initialise/de-initialise libraries cleanly and safely using RAII (constructor/destructor)
// We can throw exceptions and catch them in `main()`, so we only catch/log errors in one place.
class FakeNews
{
    public:
    FakeNews()
    {
        if (curl_global_init(CURL_GLOBAL_DEFAULT))
            throw exc::init("Initialisation failed", "libcurl");
    }

    ~FakeNews()
    { curl_global_cleanup(); }

    // The functionality of this program should really be broken up into smaller functions.
    // This might throw anything deriving from `exc::exception`.
    void run(int argc, char* argv[]);

    private:
    // Now this is a function signaure! Runs all the estimators you give it. (You should set their
    // articles using `estimator.article()` first of course.)
    // estimators: A `map` of `pair`s. The keys are the human-readable display names of the
    //             estimators, and the values go <estimator, desired weight>.
    // Returns a map, where the keys are the display names you passed into `estimators`, and the
    // values are a pair which goes <resulting estimate, weight>. The 'weight' is the real weight
    // that got calculated, not the desired weight (see below), so they all add up to 1.
    // Throws `exc::arg` if one of the desired weights is < 0 or > 1.
    // Throws anything that the `estimate()` method on any of your estimators might throw.

    // 'Desired weight'
    // This should be a number between 0 and 1, indicating how much weight you want it to have.
    // (Numbers below 0 are interpreted as 0, and above 1 are interpreted as 1.) These are only
    // important relative to each other; if they all have the same 'desired weight' they all have
    // the same weight in the output. The 'desired weight' is used to calculate the actual weights
    // in the output. The point is to save you from having to mess around making sure all the
    // weights add up to 1. Just give it an indicator between 0 and 1, and it will scale the actual
    // weights for you, depending on the number of `Estimator`s, to make them all add up to 1.

    std::map<string, std::pair<estimator::Estimate, float>>
        estimate(const std::map<string, std::pair<estimator::Estimator*, float>>& estimators);

    // Calculates the weighted average of the veracity and confidence of a load of estimates.
    // estimates: This should be the return value of `estimate()`. (See `estimate()`.) The keys
    //            aren't actually used for anything.
    // Returns one `Estimate` containing the average veracity and confidence.
    // This function does NOT check whether the weights are a valid value. If they aren't between 0
    // and 1 and don't add up to 1, you'll just get the wrong answer.
    estimator::Estimate
        average(const std::map<string, std::pair<estimator::Estimate, float>>& estimates);

    // All the file paths.
    const string _WHITELIST     = "whitelist.txt";
    const string _BLACKLIST     = "blacklist.txt";
    const string _HITLIST       = "hitlist.txt";
    const string _WORDLIST      = "hitlist.txt";
    const string _TRAINING_DATA = "training_data.txt";
};

int main(int argc, char* argv[])
{
    try
    {
        FakeNews fn;
        fn.run(argc, argv);
    }

    catch (const exc::format& e)
    {
        log::error(e.which(), e.line(), e.column()) << e.what() << endl;
        return 1;
    }

    catch (const exc::exception& e)
    {
        log::error(e.which()) << e.what() << endl;
        return 1;
    }

    pause();
    return 0;
}

void FakeNews::run(int argc, char* argv[])
{
    // We don't bother catching any `exc::exception`s in this method. Let the caller handle them.

    string input;
    cerr << "Please enter:" << endl;
    cerr << "1) Generate training data (first time use)" << endl;
    cerr << "2) Evaluate articles" << endl;
    cerr << "3) Exit" << endl;

    while (true)
    {
        cerr << "(1/2/3) > " << flush;
        std::getline(cin, input);
        if (input.size() != 1) continue;
        
        switch (input[0])
        {
            case '1':
            {
                log::log(_TRAINING_DATA) << "Making training data..." << endl;
                string training_data = neuralnet::make_training_data(_WHITELIST, _BLACKLIST,
                    _WORDLIST, 8);
                log::log(_TRAINING_DATA) << "Writing training data..." << endl;
                std::ofstream file(_TRAINING_DATA);
                if (!file.good()) throw exc::file(fs::error(), _TRAINING_DATA);
                file << training_data << endl;
                log::success << "Everything went well!" << endl;
                return;
            }

            case '2':
                goto after_loop;
                break;
            
            case '3':
                return;
            
            default:
                continue;
        }
    }
    after_loop:

    string article_dir;

    // If there were no arguments given, prompt the user for a path.
    if (argc == 1)
    {
        cerr << "Article directory not supplied, please enter one: (Leave blank to cancel.)"
            << endl;
        cerr << "> " << std::flush;
        getline(std::cin, article_dir);
        std::cin.clear();
        if (article_dir.empty()) return;
    }
   
    // Otherwise the first argument is the path.
    else article_dir = argv[1];

    // <full path to article, article>
    std::map<string, article::Article> articles;
    
    for (const string& path : fs::get_files(article_dir))
    {
        // Is the word 'article' starting to sound weird to you?
        article::Article article = article::Article(path);
        
        // Make sure that the current article doesn't have an address we've already seen.
        for (const auto& p : articles)
        {
            if (p.second.address() == article.address())
            {
                std::stringstream ss;
                ss << "Articles '" << p.first << "' and '" << path << "' have equivalent URL"
                    << endl;
                throw exc::format(ss.str(), p.second.address().full());
            }
        }

        articles.emplace(path, std::move(article));
    }

    if (articles.empty()) return;

    log::log << "Teaching the neural network..." << endl;

    // Just to avoid typing this out for convenience/readability.
    const article::Article* first_article = &(*(articles.cbegin())).second;

    // Create all the estimators.
    log::log << "Loading blacklist and whitelist..." << endl;
    estimator::BlackWhiteEstimator blackwhitelist(first_article, _BLACKLIST, _WHITELIST);

    log::log << "Loading hitlist..." << endl;
    estimator::HitListEstimator hitlist(first_article, _HITLIST);

    log::log << "Training neural network..." << endl;
    estimator::NeuralNetEstimator neuralnet(first_article, _TRAINING_DATA, _WORDLIST);

    // Collect them together with weights for `estimate()`.
    std::map<string, std::pair<estimator::Estimator*, float>> estimators;
    estimators.emplace("blackwhitelist", std::make_pair(&blackwhitelist, 1.0f));
    estimators.emplace("hitlist", std::make_pair(&hitlist, 0.7f));
    estimators.emplace("neuralnet", std::make_pair(&neuralnet, 0.9f));

    // Now do the actual estimating.
    for (const auto& p : articles)
    {
        // Tell the estimators to use the new article.
        for (const auto& p2 : estimators) p2.second.first->article(&p.second);

        cout << "> \"" << p.first << '"' << endl;
        
        auto result = estimate(estimators);

        estimator::Estimate avg = average(result);
        cout << "veracity: " << avg.veracity << ", confidence: " << avg.confidence << endl;

        for (const auto& p2 : result)
        {
            cout << p2.first << ": veracity: " << p2.second.first.veracity << ", confidence: "
                << p2.second.first.confidence << ", weight: " << p2.second.second << endl;
        }

        cout << endl;
    }
}

std::map<string, std::pair<estimator::Estimate, float>>
    FakeNews::estimate(const std::map<string, std::pair<estimator::Estimator*, float>>& estimators)
{
    std::map<string, std::pair<estimator::Estimate, float>> result;

    // Do the estimating. This is readable, lol. Weights are 0 for now.
    for (const auto& p : estimators)
        result.emplace(p.first, std::make_pair(p.second.first->estimate(), 0.0f));

    // Now calculate the weights. We do this after estimating because we want to exclude results
    // with a confidence of 0.

    // Find the sum of all the weights, not counting those with a confidence of 0, checking for
    // invalid values as we go.
    float sum = 0;

    for (const auto& p : estimators)
    {
        if (p.second.second > 1 || p.second.second < 0)
        {
            std::stringstream ss;
            ss << "Invalid weight '" << to_string(p.second.second) << "' for estimator '" << p.first
                << "; must be >= 0 and <= 1";
            throw exc::arg(ss.str(), "estimators");
        }

        if (result[p.first].first.confidence != 0.0f) sum += p.second.second;
    }

    // Now use the sum to calculate the final weight (just desired / sum), but making it 0 if the
    // confidence is 0.
    for (const auto& p : estimators)
    {
        auto& pair = result[p.first];
        pair.second = pair.first.confidence == 0.0f ? 0.0f : p.second.second / sum;
    }

    return result;
}

estimator::Estimate
    FakeNews::average(const std::map<string, std::pair<estimator::Estimate, float>>& estimates)
{
    estimator::Estimate result = estimator::Estimate { 0, 0 };

    for (const auto& p : estimates)
    {
        // Ignore results with a confidence of 0.
        if (p.second.first.confidence != 0.0f)
        {
            result.confidence += p.second.first.confidence * p.second.second;
            result.veracity   += p.second.first.veracity * p.second.second;
        }
    }

    return result;
}