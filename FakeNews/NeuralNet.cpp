// Source: David Miller
//https://www.youtube.com/watch?v=KkwX7FkLfug
// 'tutorial' video used for code production


#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
using namespace std;
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
Training::Training(const string filename)
{
	n_trainingDataFile.open(filename.c_str());
}
unsigned Training::getNextInputs(vector<double> &inputVals)
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
unsigned Training::getTargetOutputs(vector<double> &targetOutputVals)
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
int main()
{
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
		cout << "Net recent average error: "
			<< myNetwork.getRecentAverageError() << endl;
	}
	cout << endl << "Done" << endl;
	system("pause");
}
