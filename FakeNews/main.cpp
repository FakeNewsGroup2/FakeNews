#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <memory>

#include <cstdlib>

#include "curl/curl.h"
//include "curl/types.h"
#include "curl/easy.h"
#include <cstdio>

#include "fs.h"
#include "exc.h"
#include "net.h"
#include "Article.h"
#include "BlackWhiteEstimator.h"
#include "NeuralNet.h"
#include "HitListEstimator.h"

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
//#include <fstream>
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
using std::cerr;
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
    { if (curl_global_init(CURL_GLOBAL_DEFAULT)) throw exc::init("Could not initialise libcurl"); }

    ~FakeNews()
    { curl_global_cleanup(); }

    // The functionality of this program should really be broken up into smaller functions.
    // This might throw anything deriving from `exc::exception`.
    void run(int argc, char* argv[]);

    private:
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

    catch (const exc::exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}


void FakeNews::run(int argc, char* argv[])
{
    // We don't bother catching any `exc::exception`s in this method. Let the caller handle them.
    vector<net::Address> addresses;

    for (int i = 1; i < argc; ++i)
        addresses.emplace_back(argv[i]);

    // If there were no arguments given, prompt the user for URLs.
    if (argc == 1)
    {
        cout << "Please enter URLs, one per line. (Leave blank and press Enter when done.)" << endl;
        
        for (int i = 1; ; ++i)
        {
            string line;
            cout << "(URL " << i << ") > " << std::flush;
            getline(std::cin, line);
            if (line.empty()) break;
            addresses.emplace_back(line);
        }

        cout << endl;
    }

    // TODO Do stuff!

    // TODO Create and use a load of `Estimator`s on all the URLs, weight their estimates and
    // produce an average confidence.

    // Just manually create a test article for now.
    article::Article test_article
    (
        "Breaking News: Group 2 Passes Assignment",
        "Due to some absolutely gr8 teamwork, Group 2 have passed their assignment.",
        net::Address("http://www.legit_site.com/articles/group2.html")
    );

    // Evaluate it using black/whitelists.
    // We use a pointer because there is no default constructor and we want to wrap the creation in
    // a try block.
    estimator::BlackWhiteEstimator bwe(&test_article, "blacklist.txt", "whitelist.txt");
    estimator::Estimate result = bwe.estimate();

    cout << "For our test article, veracity is '" << result.veracity << ",' and confidence is '"
        << result.confidence << ".'" << endl;

    cout << "veracity:   0 = definitely fake, 1 = definitely true" << endl;
    cout << "confidence: 0 = can't estimate,  1 = we are certain"  << endl;

    if (!addresses.empty())
    {
        article::Article input_one
        (
            "PLACEHOLDER HEADLINE",
            "PLACEHOLDER CONTENT",
            addresses[0]
        );

        result = bwe.article(&input_one).estimate();

        cout << "For input one, veracity is '" << result.veracity << ",' and confidence is '"
            << result.confidence << ".'" << endl;
    }

    net::Address hl_article = net::Address("http://eelslap.com");

    article::Article hit_list_test
    (
        "PLACEHOLDER HEADLINE",
        net::get_file(hl_article.full()),
        hl_article
    );

    estimator::HitListEstimator hle(&hit_list_test, "HitList.txt");
    result = hle.estimate();

    cout << "For the hitlist test (" << hl_article.full() << "), veracity is '" << result.veracity
        << ",' and confidence is '" << result.confidence << ".'" << endl;
	
    pause();
	
    // TODO Put this away somewhere.
	neuralnet::Training trainData("trainingData.txt");
	vector<unsigned> Structure;
	trainData.getStructure(Structure);
	neuralnet::Network myNetwork(Structure);
	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;
	while (!trainData.isEof()) {
		++trainingPass;
		cout << "\nPass " << trainingPass << endl;
		if (trainData.getNextInputs(inputVals) != Structure[0]) break;
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

	cout << endl << "Done" << endl;

    pause();
}
