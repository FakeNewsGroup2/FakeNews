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
    // Demonstrating the new loggers.
    // There is `log::log`, `log::error`, `log::warning` and `log::success`

    try
    {
        FakeNews fn;
        fn.run(argc, argv);
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
    vector<net::Address> addresses;

    string path = "articles/test1.txt";
    log::log(path) << "Loading article..." << endl;
    article::Article article(path);
    log::success(path) << "Loaded OK." << endl;

    cout << "Address: " << article.address().full() << endl;
    cout << "Headline: " << article.headline() << endl;
    cout << "Contents:\n" << article.contents() << endl;

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
		for (auto attack = sites.begin(); attack != sites.end(); ++attack)
		{
			
			string http_content;
			string line;
			cout << "Stage 2 - Get HTML content and check for suspicious content" << endl;

			
			line = sites[count];
			cout << line << endl;
			http_content = net::get_file(line); 
			//catch (const exc::exception& e) { die(e.what()); }

			// (Make the page contents upper case, then make the search terms upper case, and bingo,
			// case-insensitive search.)
			for (char& c : http_content) c = toupper(c);

			// Load the search terms from a file, one per line.
			vector<string> hit_list;

			string path = "BlackHitList.txt";
			ifstream file(path);

			// TODO Better error message.
			//if (!file.good()) die(string("Could not open file '") + path + string("' for reading.'"));

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
