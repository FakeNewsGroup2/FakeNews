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

    // Makes the training data for the neural network from whitelisted sites, blacklisted sites, and
    // a hitlist of words to look for. Also repeats the training data, shuffling all the articles
    // into a random order.
    // whitelist_path: The path to the whitelist file to load.
    // blacklist_path: The path to the blacklist file to load.
    // hitlist_path:   The path to the hitlist file to load.
    // repetitions:    The number of times to repeat the training data. (Including the first, so '3'
    //                 means the data occurs 3 times total.)
    // Returns a string containing the training data. The caller can write this to a file wherever
    // it wants. 
    // Throws anything `load_clean_warn()` throws.
    // Throws anything `net::Address(const string&)` throws.
    // Throws `exc::format()` if a hitlist entry contains any spaces or punctuation. (Hyphens are
    // allowed, leading/trailing whitespace is ignored.)
    string make_training_data(const string& whitelist_path, const string& blacklist_path, const
        string& hitlist_path, int repetitions);

    // Makes a line of training data by counting words from a hitlist.
    // page:    The page to count words on.
    // hitlist: A list of words.
    // Returns the 'in' line.
    // TODO Make `page` an `Article`.
    string training_line(const string& page, const vector<string>& hitlist);

    // Loads the training data from a file, adding all the articles to the bottom of it.
    // training_path: The path to the training data file.
    // hitlist_path:  The path to the hitlist file.
    // articles:      The articles to add to the bottom. It's a map because it's a map in `run()`
    //                and I can't be bothered to change it, so there. The keys aren't actually
    //                touched.
    // Throws `exc::format` if the topology could not be read from the training data file. It does
    // NOT throw anything if the rest of the file is wrong. (It doesn't check.)
    // Throws `exc::format` if the topology of the training data does not equal the size of the
    // hitlist.
    // Throws anything `fs::load_lines()` throws.
    string load_training_data(const string& training_path, const string& hitlist_path,
        const map<string, article::Article>& articles);

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

    // All the file paths.
    const string _WHITELIST     = "whitelist.txt";
    const string _BLACKLIST     = "blacklist.txt";
    const string _HITLIST       = "hitlist.txt";
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
                log::log(_TRAINING_DATA) << "Making training data..." << endl;
                string training_data = make_training_data(_WHITELIST, _BLACKLIST, _HITLIST, 8);
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

    string training_data = load_training_data(_TRAINING_DATA, _HITLIST, articles);
    
    {
        std::ofstream test_output(R"end(C:\Users\Ethan\Desktop\test_output.txt)end");
        test_output << training_data;
    }

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

        myNetwork.feedForward(inputVals);
		myNetwork.getResults(resultVals);
		trainData.getTargetOutputs(targetVals);
        assert(targetVals.size() == Structure.back());
		myNetwork.backProp(targetVals);

        // Only print every 250 passes.
        if (!(trainingPass % 250))
        {
            cout << "\nPass " << trainingPass << endl;
            display_vector("Inputs:", inputVals);
            display_vector("Outputs:", resultVals);
            display_vector("Targets:", targetVals);
            cout << "Net recent average error: " << myNetwork.getRecentAverageError() << endl;
        }
	}

    // Print the last pass.
    cout << "\nPass " << trainingPass << endl;
    display_vector("Inputs:", inputVals);
    display_vector("Outputs:", resultVals);
    display_vector("Targets:", targetVals);
    cout << "Net recent average error: " << myNetwork.getRecentAverageError() << endl;

    // TODO The last articles.size() passes are the ones whose results we care about. Get them and
    // use them. Or perhaps we should only use the last pass and do the whole learning again every
    // time.

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
    const string& hitlist_path, int repetitions)
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

    std::stringstream ss;

    ss << "topology: " << hitlist.size() << ' ' << hitlist.size() * 2 << " 1";

    // We want to repeat the training data. Therefore we push each 'piece' (i.e. an 'in' line
    // followed by an 'out' line) to this multiple times, then shuffle the pieces.
    vector<std::pair<string, string>> pieces;

    // TODO Maybe if it turns out to be easy, you could multithread this.

    for (const net::Address& site : whitelist)
    {
        log::log(site.full()) << "Getting whitelisted page..." << endl;
        
        string html;
        try                       { html = util::upper(net::get_file(site.full())); }
        catch (const exc::net& e) { log::error(e.which()) << e.what() << endl;      }

        for (int i = 0; i < repetitions; ++i)
            pieces.emplace_back(std::move(training_line(html, hitlist)), "out: 1.0");
    }

    for (const net::Address& site : blacklist)
    {
        log::log(site.full()) << "Getting blacklisted page..." << endl;
        
        string html;
        try                       { html = util::upper(net::get_file(site.full())); }
        catch (const exc::net& e) { log::error(e.which()) << e.what() << endl;      }

        for (int i = 0; i < repetitions; ++i)
            pieces.emplace_back(std::move(training_line(html, hitlist)), "out: 0.0");
    }

    // Now shuffle the pieces and add them.
    util::shuffle(pieces);
    for (std::pair<string, string>& p : pieces)
        ss << endl << std::move(p.first) << endl << std::move(p.second);

    return ss.str();
}

string FakeNews::training_line(const string& page, const vector<string>& hitlist)
{
    string result = "in:";
    string page_upper = util::upper(page);

    for (const string& word : hitlist)
        result += page.find(util::upper(word)) == string::npos ? " 0.0" : " 1.0";

    return result;
}

string FakeNews::load_training_data(const string& training_path, const string& hitlist_path,
        const map<string, article::Article>& articles)
{
    // Load the training data. We do it this way to make sure the code is line-ending-agnostic.
    string training_data;

    {
        std::stringstream ss;
        vector<string> lines = fs::load_lines(training_path);
        
        if (!lines.empty()) ss << std::move(lines[0]);
        
        for (decltype(lines.size()) i = 1; i < lines.size(); ++i)
            ss << endl << std::move(lines[i]);

        training_data = ss.str();
    }

    // Here we find out what the topology is, so we can check it against the size of the hitlist.
    // (Which we load soon.) If they are different, then we have a problem. (Because the hit list is
    // newer than the training data.) Otherwise, everything's OK. (Actually it's not because the
    // hitlist could be different but the same length, but oh well. I have a deadline to meet.)

    static std::regex r(R"end(^topology: ([[:digit:]]+))end");
    std::smatch m;

    // If the training data is in the wrong format, explode.
    if (!std::regex_search(training_data, m, r))
        throw exc::format("Could not load training data: 'topology' line not present",
            training_path, 1);

    // Otherwise, since it consists of only digits, it should be safe to convert straight to an int.
    // Of course this will fail if the topology is some obscenely large number, but who cares.
    int topology = std::stoi(m[1]);

    // TODO Use load_check_warn or whatever it was. And check if I accidentally used load_lines
    // anywhere else.
    vector<string> hitlist = fs::load_lines(hitlist_path);

    if (topology != hitlist.size())
        throw exc::format("Training data topology does not match hitlist size."
            " (Probably hitlist is newer than training data)", training_path, 1);

    // Now we add the current articles to the end of the training data.
    std::stringstream ss;
    ss << std::move(training_data);
    
    for (const auto& p : articles)
        ss << endl << training_line(p.second.contents(), hitlist) << endl << "out: 0.0";

    return ss.str();
}