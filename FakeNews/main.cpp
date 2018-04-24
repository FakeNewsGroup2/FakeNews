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

#include "log.h"
#include "fs.h"
#include "exc.h"
#include "net.h"
#include "Article.h"
#include "BlackWhiteEstimator.h"
#include "NeuralNet.h"
#include "HitListEstimator.h"

#include <map>
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

    catch (const exc::format& e)
    {
        log::error(e.path(), e.line(), e.column()) << e.what() << endl;
        return 1;
    }

    catch (const exc::file& e)
    {
        log::error(e.path()) << e.what() << endl;
        return 1;
    }

    catch (const exc::exception& e)
    {
        log::error << e.what() << endl;
        return 1;
    }

    return 0;
}

void FakeNews::run(int argc, char* argv[])
{
    // We don't bother catching any `exc::exception`s in this method. Let the caller handle them.

    string article_dir;

    // If there were no arguments given, prompt the user for a path.
    if (argc == 1)
    {
        cout << "Please enter path to a directory of articles. (Leave blank to cancel.)" << endl;
        cout << "> " << std::flush;
        getline(std::cin, article_dir);
        if (article_dir.empty()) return;
    }
   
    // Otherwise the first argument is the path.
    else article_dir = argv[1];

    std::map<string, article::Article> articles;
    
    for (const string& path : fs::get_files(article_dir))
        articles.emplace(path, std::move(article::Article(path)));

    for (const auto& v : articles)
    {
        cout << v.first << endl;
        cout << v.second.address().full() << endl;
        cout << v.second.headline() << endl;
    }

    // Test article is just the first article we loaded, whatever that was.
    // (Actually it's a `std::pair<string, Article>` where the `string` is the file path.)
    if (articles.empty()) return; // This is just to stop it crashing if we haven't loaded any.
    const auto& test_article = *articles.begin();

    // Make a `HitListEstimator` just to test the error messages when it's the wrong format.
    log::log(test_article.first) << "Testing article..." << endl;
    estimator::HitListEstimator hle(&test_article.second, "hitlist.txt");
    estimator::Estimate estimate = hle.estimate();
    cout << "Veracity:   " << estimate.veracity << endl;
    cout << "Confidence: " << estimate.confidence << endl;
    
    log::success << "Everything went well!" << endl;
    return;

	string s;
	int NumberOfInputs = 0;

	ifstream in;
	in.open("BlackHitList.txt");

	while (!in.eof()) {
		getline(in, s);
		NumberOfInputs++;
	}

		std::ifstream file("BlackHitList.txt");
		std::string str;
		vector<string> file_contents;
		while (std::getline(file, str))
		{
			
			file_contents.push_back(str);
		}
	

		std::ifstream file2("Whitelist2.txt");
		string trainingdata = "TrainingData2.txt";
		std::string str2;
		vector<string> sites;
		while (std::getline(file2, str2))
		{
			sites.push_back(str2);
		}

		ofstream myfile;
		myfile.open(trainingdata);
		myfile << "topology: ";
		myfile << NumberOfInputs;
		myfile << " ";
		myfile << (NumberOfInputs) * 2;
		myfile << " ";
		myfile << "1\nin: ";
		
		myfile.close();
		int count = 0;
        for (auto attack : sites)
		{
			
			string http_content;
			string line;
			cout << "Stage 2 - Get HTML content and check for suspicious content" << endl;

			
			line = sites[count];
			cout << line << endl;
			http_content = net::get_file(line);

			// (Make the page contents upper case, then make the search terms upper case, and bingo,
			// case-insensitive search.)
			for (char& c : http_content) c = toupper(c);

			// Load the search terms from a file, one per line.
			vector<string> hit_list;

			string path = "hitlist.txt";
			ifstream file(path);

			if (!file.good()) throw exc::file(fs::error(), path);

			// Load each search term, converting to upper case as we go.
			for (string line; std::getline(file, line);)
			{
				for (char& c : line) c = toupper(c);
				hit_list.emplace_back(line);
			}

			// For each word in `hit_list`, search for it, and count it once if any occurrences are found.
			size_t hits = 0;
			std::ofstream out;

			out.open(trainingdata, std::ios_base::app);
			out << "\nin: ";
			for (const string& s : hit_list)
			{
				if (http_content.find(s) != string::npos)
				{
					++hits;
					std::ofstream outfile;

					outfile.open(trainingdata, std::ios_base::app);
					outfile << "1.0 ";
					continue;
				}
				else
				{
					std::ofstream outfile;

					outfile.open(trainingdata, std::ios_base::app);
					outfile << "0.0 ";
				}
			}
			std::ofstream outfile2;

			outfile2.open(trainingdata, std::ios_base::app);
			outfile2 << "\nout: 1.0";

			cout << "HitList word matches: " << hits;
			count++;
		}

		
    // TODO Do stuff!

    // TODO Create and use a load of `Estimator`s on all the URLs, weight their estimates and
    // produce an average confidence.
	
    // TODO Put this away somewhere.
	neuralnet::Training trainData("training_data.txt");
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

    cout << '\n';
	log::success << "Everything went well!" << endl;
    pause();
}
