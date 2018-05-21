// Source: David Miller
// https://www.youtube.com/watch?v=KkwX7FkLfug
// 'tutorial' video used for code production

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

#include "NeuralNet.h"
#include "log.h"
#include "net.h"
#include "util.h"
#include "exc.h"

using namespace std;

namespace fakenews
{

namespace neuralnet
{

string make_training_data(const string& whitelist_path, const string& blacklist_path,
    const string& wordlist_path, int repetitions)
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
    
    vector<string> wordlist = util::load_words(wordlist_path);

    std::stringstream ss;

    ss << "topology: " << wordlist.size() << ' ' << wordlist.size() * 2 << " 1";

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
            pieces.emplace_back(std::move(training_line(html, wordlist)), "out: 1.0");
    }

    for (const net::Address& site : blacklist)
    {
        log::log(site.full()) << "Getting blacklisted page..." << endl;
        
        string html;
        try                       { html = util::upper(net::get_file(site.full())); }
        catch (const exc::net& e) { log::error(e.which()) << e.what() << endl;      }

        for (int i = 0; i < repetitions; ++i)
            pieces.emplace_back(std::move(training_line(html, wordlist)), "out: 0.0");
    }

    // Now shuffle the pieces and add them.
    util::shuffle(pieces);
    for (std::pair<string, string>& p : pieces)
        ss << endl << std::move(p.first) << endl << std::move(p.second);

    return ss.str();
}

string training_line(const string& page, const vector<string>& wordlist)
{
    string result = "in:";
    string page_upper = util::upper(page);

    for (const string& word : wordlist)
        result += page.find(util::upper(word)) == string::npos ? " 0.0" : " 1.0";

    return result;
}

void Training::getStructure(vector<unsigned> &Structure)
{
	string line;
	string label;
	getline(n_trainingDataFile, line);
	stringstream ss(line);
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

size_t Training::getNextInputs(vector<double> &inputVals)
{
	inputVals.clear();
	string line;
	getline(n_trainingDataFile, line);
	stringstream ss(line);
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
size_t Training::getTargetOutputs(vector<double> &targetOutputVals)
{
	targetOutputVals.clear();
	string line;
	getline(n_trainingDataFile, line);
	stringstream ss(line);
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
double A_Neuron::sumDOW(const Layer &nextLayer) const {double sum = 0.0;
	for (unsigned n = 0; n < nextLayer.size() - 1; ++n) {sum += n_outputWeights[n].nodeweight * nextLayer[n].n_gradient;}
	return sum;}
void A_Neuron::calculateHiddenGradients(const Layer &nextLayer){double dow = sumDOW(nextLayer);n_gradient = dow * A_Neuron::transferFunctionDerivative(n_outputVal);}
void A_Neuron::calculateOutputGradients(double targetVal){double delta = targetVal - n_outputVal;n_gradient = delta * A_Neuron::transferFunctionDerivative(n_outputVal);}
double A_Neuron::transferFunction(double x){return tanh(x);}
double A_Neuron::transferFunctionDerivative(double x){return 1.0 - x * x;}
void A_Neuron::feedForward(const Layer &prevLayer)
{
	double sum = 0.0;
	for (unsigned n = 0; n < prevLayer.size(); ++n) {
		sum += prevLayer[n].getOutputVal() *
			prevLayer[n].n_outputWeights[n_myIndex].nodeweight;
	}

	n_outputVal = A_Neuron::transferFunction(sum);
}
A_Neuron::A_Neuron(unsigned numOutputs, unsigned myIndex):
    n_outputVal(0),
    n_outputWeights(),
    n_myIndex(0),
    n_gradient(0)
{
	for (unsigned c = 0; c < numOutputs; ++c) {
		n_outputWeights.push_back(Weights());
		n_outputWeights.back().nodeweight = randomWeight();
	}
	n_myIndex = myIndex;
}

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
	for (size_t layerNum = n_layers.size() - 2; layerNum > 0; --layerNum) {
		Layer &hiddenLayer = n_layers[layerNum];
		Layer &nextLayer = n_layers[layerNum + 1];

		for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
			hiddenLayer[n].calculateHiddenGradients(nextLayer);
		}
	}
	for (size_t layerNum = n_layers.size() - 1; layerNum > 0; --layerNum) {
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
Network::Network(const vector<unsigned> &Structure):
    n_layers(),
    n_error(0),
    n_recentAverageError(0)
{
	size_t numLayers = Structure.size();
	for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum) {
		n_layers.push_back(Layer());
		unsigned numOutputs = layerNum == Structure.size() - 1 ? 0 : Structure[layerNum + 1];
		for (unsigned neuronNum = 0; neuronNum <= Structure[layerNum]; ++neuronNum) {
			n_layers.back().push_back(A_Neuron(numOutputs, neuronNum));
			// log::log << "Neuron created" << endl;
		}
		n_layers.back().back().setOutputVal(1.0);
	}
}

}

}
