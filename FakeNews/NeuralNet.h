#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace fakenews
{

namespace neuralnet
{

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
	Training(const std::string &filename);
	bool isEof(void) { return n_trainingDataFile.eof(); }
	void getStructure(std::vector<unsigned> &structure);
	size_t getNextInputs(std::vector<double> &inputVals);
	size_t getTargetOutputs(std::vector<double> &targetOutputVals);
private:
	std::ifstream n_trainingDataFile;
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