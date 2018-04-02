#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable : 4996)

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

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}

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


	class Training
	{
	public:
		Training(const string filename);
		bool isEof(void) { return n_trainingDataFile.eof(); }
		void getStructure(vector<unsigned> &structure);
		unsigned getNextInputs(vector<double> &inputVals);
		unsigned getTargetOutputs(vector<double> &targetOutputVals);
	private:
		ifstream n_trainingDataFile;
	};

	void Training::getStructure(vector<unsigned> &Structure)
	{
		string line;
		string label;
		getline(n_trainingDataFile, line);
		std::stringstream ss(line);
		ss >> label;
		if (this->isEof() || label.compare("topology:") != 0) {
			abort();
		}
		while (!ss.eof()) {
			unsigned n;
			ss >> n;
			Structure.push_back(n);
		}
		return;
	}
	Training::Training(const string filename)
	{
		n_trainingDataFile.open(filename.c_str());
	}
	unsigned Training::getNextInputs(vector<double> &inputVals)
	{
		inputVals.clear();
		string line;
		getline(n_trainingDataFile, line);
		std::stringstream ss(line);
		string label;
		ss >> label;
		if (label.compare("in:") == 0) {
			double oneValue;
			while (ss >> oneValue) {
				inputVals.push_back(oneValue);
			}
		}
		return inputVals.size();
	}
	unsigned Training::getTargetOutputs(vector<double> &targetOutputVals)
	{
		targetOutputVals.clear();
		string line;
		getline(n_trainingDataFile, line);
		std::stringstream ss(line);
		string label;
		ss >> label;
		if (label.compare("out:") == 0) {
			double oneValue;
			while (ss >> oneValue) {
				targetOutputVals.push_back(oneValue);
			}
		}
		return targetOutputVals.size();
	}
	struct Weights
	{
		double nodeweight;
		double nodedeltaWeight;
	};
	class A_Neuron;
	typedef vector<A_Neuron> Layer;
	class A_Neuron
	{
	public:
		A_Neuron(unsigned numOutputs, unsigned myIndex);
		void setOutputVal(double val) { n_outputVal = val; }
		double getOutputVal(void) const { return n_outputVal; }
		void feedForward(const Layer &prevLayer);
		void calculateOutputGradients(double targetVal);
		void calculateHiddenGradients(const Layer &nextLayer);
		void updateInputWeights(Layer &prevLayer);

	private:
		static double eta;
		static double alpha;
		static double transferFunction(double x);
		static double transferFunctionDerivative(double x);
		static double randomWeight(void) { return rand() / double(RAND_MAX); }
		double sumDOW(const Layer &nextLayer) const;
		double n_outputVal;
		vector<Weights> n_outputWeights;
		unsigned n_myIndex;
		double n_gradient;
	};
	double A_Neuron::eta = 0.15;
	double A_Neuron::alpha = 0.5;
	void A_Neuron::updateInputWeights(Layer &prevLayer)
	{
		for (unsigned n = 0; n < prevLayer.size(); ++n) {
			A_Neuron &neuron = prevLayer[n];
			double oldDeltaWeight = neuron.n_outputWeights[n_myIndex].nodedeltaWeight;
			double newDeltaWeight = eta * neuron.getOutputVal() * n_gradient + alpha * oldDeltaWeight;
			neuron.n_outputWeights[n_myIndex].nodedeltaWeight = newDeltaWeight;
			neuron.n_outputWeights[n_myIndex].nodeweight += newDeltaWeight;
		}
	}
	double A_Neuron::sumDOW(const Layer &nextLayer) const {
		double sum = 0.0;
		for (unsigned n = 0; n < nextLayer.size() - 1; ++n) { sum += n_outputWeights[n].nodeweight * nextLayer[n].n_gradient; }
		return sum;
	}
	void A_Neuron::calculateHiddenGradients(const Layer &nextLayer) { double dow = sumDOW(nextLayer); n_gradient = dow * A_Neuron::transferFunctionDerivative(n_outputVal); }
	void A_Neuron::calculateOutputGradients(double targetVal) { double delta = targetVal - n_outputVal; n_gradient = delta * A_Neuron::transferFunctionDerivative(n_outputVal); }
	double A_Neuron::transferFunction(double x) { return tanh(x); }
	double A_Neuron::transferFunctionDerivative(double x) { return 1.0 - x * x; }
	void A_Neuron::feedForward(const Layer &prevLayer)
	{
		double sum = 0.0;
		for (unsigned n = 0; n < prevLayer.size(); ++n) {
			sum += prevLayer[n].getOutputVal() *
				prevLayer[n].n_outputWeights[n_myIndex].nodeweight;
		}

		n_outputVal = A_Neuron::transferFunction(sum);
	}
	A_Neuron::A_Neuron(unsigned numOutputs, unsigned myIndex)
	{
		for (unsigned c = 0; c < numOutputs; ++c) {
			n_outputWeights.push_back(Weights());
			n_outputWeights.back().nodeweight = randomWeight();
		}
		n_myIndex = myIndex;
	}
	class Network
	{
	public:
		Network(const vector<unsigned> &Structure);
		void feedForward(const vector<double> &inputVals);
		void backProp(const vector<double> &targetVals);
		void getResults(vector<double> &resultVals) const;
		double getRecentAverageError(void) const { return n_recentAverageError; }

	private:
		vector<Layer> n_layers;
		double n_error;
		double n_recentAverageError;
		static double n_recentAverageSmoothingFactor;
	};
	double Network::n_recentAverageSmoothingFactor = 100.0;
	void Network::getResults(vector<double> &resultVals) const
	{
		resultVals.clear();
		for (unsigned n = 0; n < n_layers.back().size() - 1; ++n) {
			resultVals.push_back(n_layers.back()[n].getOutputVal());
		}
	}
	void Network::backProp(const vector<double> &targetVals)
	{
		Layer &outputLayer = n_layers.back();
		n_error = 0.0;
		for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
			double delta = targetVals[n] - outputLayer[n].getOutputVal();
			n_error += delta * delta;
		}
		n_error /= outputLayer.size() - 1;
		n_error = sqrt(n_error);
		n_recentAverageError =
			(n_recentAverageError * n_recentAverageSmoothingFactor + n_error)
			/ (n_recentAverageSmoothingFactor + 1.0);
		for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
			outputLayer[n].calculateOutputGradients(targetVals[n]);
		}
		for (unsigned layerNum = n_layers.size() - 2; layerNum > 0; --layerNum) {
			Layer &hiddenLayer = n_layers[layerNum];
			Layer &nextLayer = n_layers[layerNum + 1];

			for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
				hiddenLayer[n].calculateHiddenGradients(nextLayer);
			}
		}
		for (unsigned layerNum = n_layers.size() - 1; layerNum > 0; --layerNum) {
			Layer &layer = n_layers[layerNum];
			Layer &prevLayer = n_layers[layerNum - 1];
			for (unsigned n = 0; n < layer.size() - 1; ++n) {
				layer[n].updateInputWeights(prevLayer);
			}
		}
	}
	void Network::feedForward(const vector<double> &inputVals)
	{
		assert(inputVals.size() == n_layers[0].size() - 1);
		for (unsigned i = 0; i < inputVals.size(); ++i) {
			n_layers[0][i].setOutputVal(inputVals[i]);
		}
		for (unsigned layerNum = 1; layerNum < n_layers.size(); ++layerNum) {
			Layer &prevLayer = n_layers[layerNum - 1];
			for (unsigned n = 0; n < n_layers[layerNum].size() - 1; ++n) {
				n_layers[layerNum][n].feedForward(prevLayer);
			}
		}
	}
	Network::Network(const vector<unsigned> &Structure)
	{
		unsigned numLayers = Structure.size();
		for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum) {
			n_layers.push_back(Layer());
			unsigned numOutputs = layerNum == Structure.size() - 1 ? 0 : Structure[layerNum + 1];
			for (unsigned neuronNum = 0; neuronNum <= Structure[layerNum]; ++neuronNum) {
				n_layers.back().push_back(A_Neuron(numOutputs, neuronNum));
				cout << "Neuron created" << endl;
			}
			n_layers.back().back().setOutputVal(1.0);
		}
	}
	void showVectorVals(string label, vector<double> &v)
	{
		cout << label << " ";
		for (unsigned i = 0; i < v.size(); ++i) {
			cout << v[i] << " ";
		}
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
	article::Article input_one
	(
		"Breaking News: Group 2 Passes Assignment",
		"Due to some absolutely gr8 teamwork, Group 2 have passed their assignment.",
		net::Address(addresses[0])
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
			(new estimator::BlackWhiteEstimator(&input_one, "blacklist.txt", "whitelist.txt"));
	}

	catch (const exc::exception& e) { die(e.what()); }

	estimator::Estimate result2 = bwe2->estimate();

	cout << "For input one, veracity is '" << result2.veracity << ",' and confidence is '"
		<< result2.confidence << ".'" << endl;

    // Only create one of each `Estimator`! To change the article, do this:
    // estimator.article(my_new_article).estimate();
    // estimator.article(another_new_article).estimate();

    cleanup();
    // system("pause"); // Please no.... just press Ctrl-F5 to run the program instead.
                        // (Or run it with command prompt as nature intended...)
	system("PAUSE");
	CURL *curl;
	FILE *fp;
	CURLcode res;
	string url = "http://www.eelslap.com";
	char outfilename[FILENAME_MAX] = "./HTTPContent.txt";
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}

	
	const int hitCount = 9;
	string hitList[hitCount];
	int hits = 0;

	ifstream file("HitList.txt");
	if (file.is_open())
	{
		

		for (int i = 0; i < hitCount; ++i)
		{
			file >> hitList[i];
		}
		

	}

	string search;
	ifstream inFile;
	string line;

	inFile.open("HTTPContent.txt");

	if (!inFile) {
		cout << "Unable to open file" << endl;
		exit(1);
	}

	for (int i = 0; i < hitCount; ++i)
	{


		search = hitList[i];


		size_t pos;
		while (inFile.good())
		{
			getline(inFile, line); // get line from file
			pos = line.find(search); // search
			if (pos != string::npos) // string::npos is returned if string is not found
			{
				cout << hitList[i];
				cout << "Found!";
				hits++;
				break;
			}
		}
		
	}
	cout << "HitList word matches: " << hits;
	

	system("PAUSE");
	
	Training trainData("trainingData.txt");
	vector<unsigned> Structure;
	trainData.getStructure(Structure);
	Network myNetwork(Structure);
	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;
	while (!trainData.isEof()) {
		++trainingPass;
		cout << endl << "Pass " << trainingPass;
		if (trainData.getNextInputs(inputVals) != Structure[0]) {
			break;
		}
		showVectorVals(": Inputs:", inputVals);
		myNetwork.feedForward(inputVals);
		myNetwork.getResults(resultVals);
		showVectorVals("Outputs:", resultVals);
		trainData.getTargetOutputs(targetVals);
		showVectorVals("Targets:", targetVals);
		assert(targetVals.size() == Structure.back());
		myNetwork.backProp(targetVals);
		cout << "Net recent average error: " << myNetwork.getRecentAverageError() << endl;
	}
	cout << endl << "Done" << endl;
	system("pause");
	return 0;


}
