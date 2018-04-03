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

// TODO Instead of having `init()`, `cleanup()` and `die()`, put the whole program into a class
// which initialises/cleans up in its constructor/destructor. Then wrap everything in `main()`
// (which should be almost nothing) in a single try/catch block.

void init()
// Initialises the libraries used in this program.
// Throws `exc::init` if a library failed to initialise.
{ if (curl_global_init(CURL_GLOBAL_DEFAULT)) throw exc::init("Could not initialise libcurl"); }

// Shuts down any libraries.
void cleanup() { curl_global_cleanup(); }

void die(const string& msg)
// Prints the error message, cleans up, and quits the program. Purely for convenience (and so that
// the error messages are consistent.)
// msg: The error message to print. Should probably be `exc::exception.what()`.
{
    cerr << "Error: " << msg << endl;
    cleanup();
    exit(1);
}

template<typename T> void display_vector(const string& label, const vector<T>& vec)
{
    cout << label;
    for (const T& v : vec) cout << ' ' << v;
    cout << endl;
}

int main(int argc, char* argv[])
{
    vector<net::Address> addresses;

    try                             { init();        }
    catch (const exc::exception& e) { die(e.what()); }

    try { for (int i = 1; i < argc; ++i) addresses.emplace_back(argv[i]); }
    catch (const exc::exception& e) { die(e.what()); }

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

            try                             { addresses.emplace_back(line); }
            catch (const exc::exception& e) { die(e.what());                }
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
    // We use a `shared_ptr` so that if there's an uncaught exception, there's no memory leak.
    std::shared_ptr<estimator::BlackWhiteEstimator> bwe;

    try
    {
        bwe = std::shared_ptr<estimator::BlackWhiteEstimator>
            (new estimator::BlackWhiteEstimator(&test_article, "blacklist.txt", "whitelist.txt"));
    }

    catch (const exc::exception& e) { die(e.what()); }

    estimator::Estimate result = bwe->estimate();

    cout << "For our test article, veracity is '" << result.veracity << ",' and confidence is '"
        << result.confidence << ".'" << endl;

    cout << "veracity:   0 = definitely fake, 1 = definitely true" << endl;
    cout << "confidence: 0 = can't estimate,  1 = we are certain"  << endl;

	std::shared_ptr<estimator::BlackWhiteEstimator> bwe2;

	try
	{
		bwe2 = std::shared_ptr<estimator::BlackWhiteEstimator>
			(new estimator::BlackWhiteEstimator(&test_article, "blacklist.txt", "whitelist.txt"));
	}

	catch (const exc::exception& e) { die(e.what()); }

	estimator::Estimate result2 = bwe2->estimate();

	cout << "For input one, veracity is '" << result2.veracity << ",' and confidence is '"
		<< result2.confidence << ".'" << endl;

    // Only create one of each `Estimator`! To change the article, do this:
    // estimator.article(my_new_article).estimate();
    // estimator.article(another_new_article).estimate();


    // system("pause"); // Please no.... just press Ctrl-F5 to run the program instead.
                        // (Or run it with command prompt as nature intended...)

    // Get the page.
	string http_content;

    try                             { http_content = net::get_file("http://eelslap.com"); }
    catch (const exc::exception& e) { die(e.what());                                      }

    // (Make the page contents upper case, then make the search terms upper case, and bingo,
    // case-insensitive search.)
    for (char& c : http_content) c = toupper(c);

    // Load the search terms from a file, one per line.
    vector<string> hit_list;

    string path = "HitList.txt";
	ifstream file(path);
	
    // TODO Better error message.
    if (!file.good()) die(string("Could not open file '") + path + string("' for reading.'"));

    // Load each search term, converting to upper case as we go.
    for (string line; std::getline(file, line);)
    {
        for (char& c : line) c = toupper(c);
        hit_list.emplace_back(line);
    }

    // For each word in `hit_list`, search for it, and count it once if any occurrences are found.
    size_t hits = 0;

    for (const string& s : hit_list)
    {
        if (http_content.find(s) != string::npos)
        {
            ++hits;
            continue;
        }
    }

	cout << "HitList word matches: " << hits;
	
	system("PAUSE");
	
    // TODO Put this somewhere not in `main()`.
	neuralnet::Training trainData("trainingData.txt");
	vector<unsigned> Structure;
	trainData.getStructure(Structure);
	neuralnet::Network myNetwork(Structure);
	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;
	while (!trainData.isEof()) {
		++trainingPass;
		cout << endl << "Pass " << trainingPass;
		if (trainData.getNextInputs(inputVals) != Structure[0]) {
			break;
		}
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
	system("pause");
    cleanup();
	return 0;


}
