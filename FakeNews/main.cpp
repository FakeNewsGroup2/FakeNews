#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <memory>
#include <algorithm>

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

#include <map>
#include <cassert>
#include <cmath>
#include <sstream>

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
    // Now this is a function signaure! Evaluates an article using all the estimators you give it.
    // article:    The article to estimate.
    // estimators: A `map` of `pair`s. The keys are the human-readable display names of the
    //             estimators, and the values go <estimator, desired weight>.
    // Returns a map, where the keys are the display names you passed into `estimators`, and the
    // values are a pair which goes <resulting estimate, weight>. The 'weight' is the real weight
    // that got calculated, not the desired weight (see below), so they all add up to 1.
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
        estimate(const article::Article& article,
        const std::map<string, std::pair<estimator::Estimator*, float>>& estimators);

    // Makes the training data for the neural network.
    // whitelist_path: The path to the whitelist file to load.
    // blacklist_path: The path to the blacklist file to load.
    // hitlist_path:   The path to the hitlist file to load.
    // Returns a string containing the training data. The caller can write this to a file wherever
    // it wants. 
    // Throws anything `load_clean_warn()` throws.
    // Throws anything `net::Address(const string&)` throws.
    // Throws `exc::format()` if a hitlist entry contains any spaces or punctuation. (Hyphens are
    // allowed, leading/trailing whitespace is ignored.)
    string make_training_data(const string& whitelist_path, const string& blacklist_path, const
        string& hitlist_path);

    // Makes a line of training data by counting words from a hitlist.
    // page:    The page to count words on.
    // hitlist: A list of words.
    // Returns the 'in' line.
    // TODO Make `page` an `Article`.
    string training_line(const string& page, const vector<string>& hitlist);

    template<typename T> void display_vector(const string& label, const vector<T>& vec)
    {
        cout << label;
        for (const T& v : vec) cout << ' ' << v;
        cout << endl;
    }

    void pause()
    {
        cout << "Press Enter to continue..." << endl;
        std::cin.clear();
        std::cin.get();
    }
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

    string input;
    cout << "Please enter:" << endl;
    cout << "1) Generate training data (first time use)" << endl;
    cout << "2) Evaluate articles" << endl;
    cout << "3) Exit" << endl;

    while (true)
    {
        cout << "(1/2/3) > " << flush;
        std::getline(cin, input);
        if (input.size() != 1) continue;
        
        switch (input[0])
        {
            case '1':
            {
                // TODO Somewhere, copy the training data loads of times.
                string training_path = "training_data.txt";
                string training_data = make_training_data("blacklist.txt", "whitelist.txt",
                    "hitlist.txt");
                log::log(training_path) << "Writing training data..." << endl;
                std::ofstream file(training_path);
                if (!file.good()) throw exc::file(fs::error(), training_path);
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
        cout << "Article directory not supplied, please enter one: (Leave blank to cancel.)"
            << endl;
        cout << "> " << std::flush;
        getline(std::cin, article_dir);
        std::cin.clear();
        if (article_dir.empty()) return;
    }
   
    // Otherwise the first argument is the path.
    else article_dir = argv[1];

    // <full path to article, article>
    std::map<string, article::Article> articles;
    
    for (const string& path : fs::get_files(article_dir))
        articles.emplace(path, std::move(article::Article(path)));

    // TODO Do stuff!

    // TODO Create and use a load of `Estimator`s on all the URLs, weight their estimates and
    // produce an average confidence.
	
    string training_data = fs::read_to_string("training_data.txt");

    // TODO Put this away somewhere.
    // TODO Somewhere put the 'in' line for the article we're evaluating on the end of the training
    // data. If the training data 'in' lines have a different number of inputs to the current
    // hitlist, the hitlist has changed since the training data was generated, and we have a
    // problem.
	neuralnet::Training trainData(training_data);
	vector<unsigned> Structure;
	trainData.getStructure(Structure);
	neuralnet::Network myNetwork(Structure);
	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;
	while (!trainData.isEof()) {
		++trainingPass;
		if (trainData.getNextInputs(inputVals) != Structure[0]) break;
        cout << "\nPass " << trainingPass << endl;
        display_vector("Inputs:", inputVals);
		myNetwork.feedForward(inputVals);
		myNetwork.getResults(resultVals);
        display_vector("Outputs:", resultVals);
		trainData.getTargetOutputs(targetVals);
        display_vector("Targets:", targetVals);
		assert(targetVals.size() == Structure.back());
		myNetwork.backProp(targetVals);
		cout << "Net recent average error: " << myNetwork.getRecentAverageError() << endl;
	}

    cout << '\n';
	log::success << "Everything went well!" << endl;
    pause();
}

std::map<string, std::pair<estimator::Estimate, float>>
        FakeNews::estimate(const article::Article& article,
        const std::map<string, std::pair<estimator::Estimator*, float>>& estimators)
{
    // TODO Actually calculate the weights, you dickhead.
    std::map<string, std::pair<estimator::Estimate, float>> result;

    for (const auto& name_estim : estimators)
    {
        float weight = name_estim.second.second;
        if      (weight < 0) weight = 0;
        else if (weight > 1) weight = 1;

        result.emplace(name_estim.first, std::make_pair(name_estim.second.first->article(&article).estimate(), weight));
    }

    return result;
}

string FakeNews::make_training_data(const string& whitelist_path, const string& blacklist_path,
    const string& hitlist_path)
{
    vector<net::Address> whitelist;
    vector<net::Address> blacklist;

    // Try and load the whitelist and blacklist.
    {
        vector<string> lines = util::load_clean_warn(whitelist_path, "URLs");
        for (string& s : lines) whitelist.emplace_back(std::move(s));
        
        lines = util::load_clean_warn(blacklist_path, "URLs");
        for (string& s : lines) blacklist.emplace_back(std::move(s));
    }
    
    vector<string> hitlist = util::load_clean_warn(hitlist_path);

    // Make sure hitlist entries don't contain any spaces or punctuation.
    for (decltype(hitlist.size()) i = 0; i < hitlist.size(); ++i)
    {
        for (char c : hitlist[i])
        {
            // This is silly because it depends on the current locale and only works for ASCII, but
            // who cares. Sorry everybody who doesn't speak English. You're probably used to
            // computers hating you by now.
            
            // TODO This line number doesn't account for duplicates being removed. We need to do
            // this check before removing duplicates. Fix it.
            if (isspace(c) || (c != '-' && ispunct(c)))
                throw exc::format(string("Invalid hitlist entry '") + hitlist[i] + "'",
                    hitlist_path, i + 1);
        }
    }

    // We build it up as a string, then write it all at the end in one go. This is so that, if
    // something fails along the way, we don't overwrite the existing training data.
    std::stringstream ss;
    ss << "topology: " << hitlist.size() << ' ' << hitlist.size() * 2 << " 1";

    for (const net::Address& site : whitelist)
    {
        log::log(site.full()) << "Getting whitelisted page..." << endl;
        string html = util::upper(net::get_file(site.full()));
        ss << training_line(html, hitlist);
        ss << "\nout: 1.0";
    }

    for (const net::Address& site : blacklist)
    {
        log::log(site.full()) << "Getting blacklisted page..." << endl;
        string html = util::upper(net::get_file(site.full()));
        ss << training_line(html, hitlist);
        ss << "\nout: 0.0";
    }

    return ss.str();
}

string FakeNews::training_line(const string& page, const vector<string>& hitlist)
{
    string result = "in: ";
    string page_upper = util::upper(page);

    for (const string& word : hitlist)
        result += page.find(util::upper(word)) == string::npos ? " 0.0" : " 1.0";

    return result;
}