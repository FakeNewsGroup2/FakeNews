#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "Article.h"

namespace fakenews
{

namespace neuralnet
{
// Makes the training data for the neural network from articles and a list of words to look for.
// Also repeats the training data, shuffling all the results into a random order.
// articles:       A vector of loaded articles. Articles whose `veracity()` is unknown are just
//                 skipped.
// wordlist_path:  The path to the wordlist file to load.
// repetitions:    The number of times to repeat the training data. (Including the first, so '3'
//                 means the data occurs 3 times total.)
// Returns a string containing the training data. The caller can write this to a file wherever
// it wants. 
// Throws anything `util::load_words()` throws.
// Throws anything `Article(const string&)` throws.
std::string make_training_data(const std::string& wordlist_path,
    const std::vector<article::Article>& articles, int repetitions);

// Makes a line of training data by counting words from a wordlist.
// page:     The page to count words on.
// wordlist: A list of words.
// Returns the 'in' line.
// TODO Make `page` an `Article`.
std::string training_line(const std::string& page, const std::vector<std::string>& wordlist);

struct Weights
{
	double nodeweight;
	double nodedeltaWeight;
};

class A_Neuron;

using Layer = std::vector<A_Neuron>;

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
	std::vector<Weights> n_outputWeights;
	unsigned n_myIndex;
	double n_gradient;
};


class Training
{
public:
    Training(const std::string &trainingData): n_trainingDataFile(trainingData) { }
	bool isEof(void) { return n_trainingDataFile.eof(); }
	void getStructure(std::vector<unsigned> &structure);
	size_t getNextInputs(std::vector<double> &inputVals);
	size_t getTargetOutputs(std::vector<double> &targetOutputVals);
    
    // Please, forgive me God for what I am about to do...
    void clearStream() { n_trainingDataFile.clear(); }
    template<typename T> std::stringstream& operator<<(T& t)
    {
        n_trainingDataFile << t;
        return n_trainingDataFile;
    }

private:
	std::stringstream n_trainingDataFile;
};

class Network
{
public:
	Network(const std::vector<unsigned> &Structure);
	void feedForward(const std::vector<double> &inputVals);
	void backProp(const std::vector<double> &targetVals);
	void getResults(std::vector<double> &resultVals) const;
	double getRecentAverageError(void) const { return n_recentAverageError; }

private:
	std::vector<Layer> n_layers; 
	double n_error;
	double n_recentAverageError;
	static double n_recentAverageSmoothingFactor;
};

}

}