#pragma once

// This file contains a class which uses the neural network code to estimate the veracity of an
// article.

#include <string>
#include <vector>
#include <map>

#include "NeuralNet.h"
#include "Article.h"
#include "Estimator.h"

namespace fakenews
{

namespace estimator
{

class NeuralNetEstimator : public Estimator
{
    public:
    // NeuralNetEstimator constructor.
    // article:       Pointer to the first article to estimate.
    // training_path: Path to a file containing training data.
    // wordlist_path: Path to the list of words to look for.
    // shut_up:       If true, don't print any output. Defaults to true.
    // TODO Document exceptions.
    NeuralNetEstimator(const article::Article* article, const std::string& training_path,
        const std::string& wordlist_path, bool shut_up = true);

    ~NeuralNetEstimator()
    {
        delete _train_data;
        delete _network;
    }

    Estimate estimate();

    private:
    // Some of these are pointers because they don't have default constructors.
    std::vector<std::string> _wordlist;
    neuralnet::Training* _train_data;
    neuralnet::Network* _network;
    std::vector<unsigned> _structure;
    std::vector<double> _inputs, _targets, _outputs;
    unsigned _pass;

    // Loads the training data from a file, making sure that the topology matches the size of the
    // wordlist.
    // path: The path to the training data file.
    // Returns the training data file as a string.
    // Throws `exc::format` if the topology could not be read from the training data file. It does
    // NOT throw anything if the rest of the file is wrong. (It doesn't check.)
    // Throws `exc::format` if the topology of the training data does not equal the size of the
    // wordlist.
    // Throws anything `fs::load_lines()` throws.
    std::string load_training_data(const std::string& path);

    // Do one training pass.
    // Returns false if there was some problem, true otherwise.
    bool do_pass();

    // Prints the number of the current pass along with the outputs/targets/recent error.
    // percent: The % complete that the whole operation is, to be printed in the message, fixed to 2
    //          decimal places. If this is -1, don't print the % complete. Defaults to -1.
    void print_pass(float percent = -1);
};

}

}