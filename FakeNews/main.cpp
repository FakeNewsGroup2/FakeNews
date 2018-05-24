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

    // Generates a word frequency list for training the neural network. It just takes all the unique
    // words from every article, orders them by frequency of occurrence, and removes the 5% most
    // common ones.
    // articles: The articles to analyse.
    // Returns a vector of every word in the word list in descending order of frequency.
    vector<string> generate_wordlist(const std::vector<article::Article>& articles);

    // Gets a line of input from the user.
    // inputs: The user is forced to enter something from this vector. If empty, the user can enter
    //         anything. If the user enters nothing, an empty string will be returned regardless of
    //         whether `inputs` contains any empty strings or not. (Use this for a 'cancel' option
    //         or similar.) Defaults to empty.
    // Returns the string that the user entered.
    string get_input(const vector<string>& inputs = {});

    // Loads every article in a folder, giving each one a particular veracity.
    // path:     The path to a directory full of articles.
    // veracity: The veracity of every article.
    // Returns a map where the keys are the file paths, and the values are the articles themselves.
    // Throws `exc::format` if two articles have an equivalent URL.
    // Throws anything `fs::get_files()` throws.
    // Throws anything `article::Article(const string&)` throws.
    map<string, article::Article> load_articles(const string& path,
        article::ArticleVeracity veracity = article::VERACITY_UNKNOWN);

    // Ask the user whether or not to continue with a 'y/n' prompt.
    // Returns true if the user answered 'y,' false if they answered 'n.'
    bool cont();

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

    return 0;
}

void FakeNews::run(int argc, char* argv[])
{
    // We don't bother catching any `exc::exception`s in this method. Let the caller handle them.

    cerr << "Please enter:" << endl;
    cerr << "1) Evaluate articles" << endl;
    cerr << "2) Generate training data (first time use)" << endl;
    cerr << "3) Generate wordlist (first time use)" << endl;
    cerr << "4) Generate hitlist (first time use)" << endl;
    cerr << "5) Exit" << endl;

    string input = get_input({ "1", "2", "3", "4", "5" });

    if (input == "1")
    {
        cerr << "Enter a path to a folder of unknown articles (leave blank to cancel):" << endl;
        string path = get_input();

        if (path.empty()) return;

        map<string, article::Article> articles = load_articles(path);

        if (articles.empty()) throw exc::file("Folder is empty", path);

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

    else if (input == "2")
    {
        cerr << "Enter a path to a folder of fake articles:" << endl;
        string fake = get_input();
        
        cerr << "Enter a path to a folder of true articles:" << endl;
        string true_ = get_input();

        if (!cont()) return;

        if (fake == true_) throw exc::file("'Fake' and 'true' article paths are the same");

        vector<article::Article> articles;

        if (!fake.empty())
        {
            log::log << "Loading fake articles..." << endl;
            map<string, article::Article> arts = load_articles(fake, article::VERACITY_FAKE);
            for (auto& p : arts) articles.emplace_back(move(p.second));
        }

        if (!true_.empty())
        {
            log::log << "Loading true articles..." << endl;
            map<string, article::Article> arts = load_articles(true_, article::VERACITY_TRUE);
            for (auto& p : arts) articles.emplace_back(move(p.second));
        }

        log::log(_TRAINING_DATA) << "Generating training data..." << endl;
        
        string training_data = neuralnet::make_training_data(_WORDLIST, articles, 20);

        log::log(_TRAINING_DATA) << "Writing training data..." << endl;
        std::ofstream file(_TRAINING_DATA);
        if (!file.good()) throw exc::file(fs::error(), _TRAINING_DATA);
        file << training_data << endl;
        log::success << "Everything went well!" << endl;
        return;
    }

    else if (input == "3")
    {
        cerr << "Enter a path to a folder of fake articles:" << endl;
        string fake = get_input();

        cerr << "Enter a path to a folder of true articles:" << endl;
        string true_ = get_input();

        if (!cont()) return;

        if (fake.empty() && true_.empty()) return;

        if (fake == true_) throw exc::file("'Fake' and 'true' article paths are the same");

        vector<article::Article> articles;

        if (!fake.empty())
        {
            log::log << "Loading fake articles..." << endl;
            map<string, article::Article> arts = load_articles(fake, article::VERACITY_FAKE);
            for (auto& p : arts) articles.emplace_back(move(p.second));
        }

        if (!true_.empty())
        {
            log::log << "Loading true articles..." << endl;
            map<string, article::Article> arts = load_articles(true_, article::VERACITY_TRUE);
            for (auto& p : arts) articles.emplace_back(move(p.second));
        }

        log::log(_WORDLIST) << "Generating wordlist..." << endl;
        vector<string> wordlist = generate_wordlist(articles);

        log::log(_WORDLIST) << "Writing wordlist..." << endl;
        std::ofstream file(_WORDLIST);
        if (!file.good()) throw exc::file(fs::error(), _WORDLIST);
        for (const string& line : wordlist) file << line << endl;
        log::success << "Everything went well!" << endl;
        return;
    }
    
    else if (input == "4")
    {
        cerr << "Enter a path to a folder of fake articles (leave blank to cancel):" << endl;
        string fake = get_input();

        if (fake.empty()) return;

        vector<article::Article> articles;

        log::log << "Loading fake articles..." << endl;
        map<string, article::Article> arts = load_articles(fake, article::VERACITY_FAKE);
        for (auto& p : arts) articles.emplace_back(move(p.second));

        log::log(_HITLIST) << "Generating hitlist..." << endl;
        vector<string> hitlist = generate_wordlist(articles);

        log::log(_HITLIST) << "Writing hitlist..." << endl;
        std::ofstream file(_HITLIST);
        if (!file.good()) throw exc::file(fs::error(), _HITLIST);
        for (const string& line : hitlist) file << line << endl;
        log::success << "Everything went well!" << endl;
        return;
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

vector<string> FakeNews::generate_wordlist(const std::vector<article::Article>& articles)
{
    vector<string> result;

    map<string, string::size_type> frequencies;

    // For every word in every article, convert it to upper case and count it.
    for (const auto& article : articles)
        for (const string& word : util::split_words(article.contents()))
            ++frequencies[util::upper(word)];

    // Convert it to a vector of pairs so we can sort it.
    vector<pair<string, string::size_type>> v;
    for (auto& p : frequencies) v.emplace_back(move(p));

    // Sort in descending order.
    sort(v.begin(), v.end(),
    [](pair<string, string::size_type>& a, pair<string, string::size_type>& b)
    {
        return a.second > b.second;
    });

    // Add the results.
    for (auto& p : v) result.emplace_back(std::move(p.first));

    // Remove the 5% most frequent.
    // TODO test this
    result.erase(result.begin(), result.begin() + (result.size() / 20));

    return result;
}

string FakeNews::get_input(const vector<string>& inputs)
{
    string input;

    while (true)
    {
        cerr << "> " << flush;
        cin.clear();
        std::getline(cin, input);
        if (input.empty()) break;
        if (inputs.empty()) break;
        if (std::find(inputs.cbegin(), inputs.cend(), input) != inputs.cend()) break;
    }

    return input;
}

map<string, article::Article> FakeNews::load_articles(const string& path,
    article::ArticleVeracity veracity)
{
    map<string, article::Article> articles;

    for (const string& file : fs::get_files(path))
    {
        // Is the word 'article' starting to sound weird to you?
        article::Article article = article::Article(file);

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

        articles.emplace(file, std::move(article));
    }

    return articles;
}

bool FakeNews::cont()
{
    cerr << "Continue? (y/n)" << endl;
    string input;
    for (; input.empty(); input = get_input({ "y", "Y", "n", "N" }));
    return input == "y" || input == "Y";
}