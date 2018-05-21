#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>
#include <regex>

#include "NeuralNetEstimator.h"

#include "Article.h"
#include "util.h"
#include "fs.h"
#include "exc.h"
#include "log.h"

using std::string;
using std::vector;
using std::map;
using std::endl;

namespace fakenews
{

namespace estimator
{

NeuralNetEstimator::NeuralNetEstimator(const article::Article* article, const string& training_path,
    const string& wordlist_path, bool shut_up):
    Estimator(article),
    _inputs(),
    _targets(),
    _results(),
    _pass(0),
    _wordlist(util::load_words(wordlist_path, "words"))
{
    string training_data = load_training_data(training_path);

    // We calculate the number of passes by counting the number of times 'in:' occurs within the
    // training data. Obviously this will mess up if the training data is wrong, but it doesn't
    // matter since we're only using it to calculate the % progress.
    auto passes = util::occurrences(training_data, "in:");

    // Do all the initial learning.
    _train_data = new neuralnet::Training(training_data);
    _train_data->getStructure(_structure);
    _network = new neuralnet::Network(_structure);

    // TODO Really this is stupid, fix this...
    if (!do_pass()) return;
    if (!shut_up) print_pass(0);

    while (!_train_data->isEof())
    {
        if (!do_pass()) return;

        // Only print every 100 passes. (And if output isn't disabled of course.)
        if (!shut_up && !(_pass % 100)) print_pass(((float)_pass / passes) * 100);
    }

    // Print the last pass.
    if(!shut_up) print_pass(100);
}

Estimate NeuralNetEstimator::estimate()
{
    // Code this stupid should be illegal...
    _train_data->clearStream();
    *_train_data << neuralnet::training_line(_article->contents(), _wordlist) << endl << "out: 0.0";
    do_pass();

    float veracity = (float)_results[0]; // _results won't be empty.... right?
    if (veracity < 0) veracity = 0;
    else if (veracity > 1) veracity = 1;

    // TODO Give a confidence estimate.
    return Estimate { veracity, 1.0 };
}

bool NeuralNetEstimator::do_pass()
{
    // TODO Handle errors properly and document them.
    ++_pass;
    if (_train_data->getNextInputs(_inputs) != _structure[0]) return false;
    _network->feedForward(_inputs);
    _network->getResults(_results);
    _train_data->getTargetOutputs(_targets);
    if (_targets.size() != _structure.back()) throw exc::exception("AAAAAHHH!!!!!");
    _network->backProp(_targets);
    return true;
}

void NeuralNetEstimator::print_pass(float percent)
{
    cout << endl;

    if (percent == -1) log::log << "Pass " << _pass << endl;

    else
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << percent << '%';
        log::log(ss.str()) << "Pass " << _pass << endl;
    }
    
    // Don't print the inputs, because it's going to be bloody long.
    // util::display_vector("inputs", _inputs);
    util::display_vector("outputs", _results);
    util::display_vector("targets", _targets);
    cout << "recent error: " << _network->getRecentAverageError() << endl;
}

string NeuralNetEstimator::load_training_data(const string& path)
{
    // Load the training data. We do it this way to make sure the code is line-ending-agnostic.
    string training_data;

    {
        std::stringstream ss;
        vector<string> lines = fs::load_lines(path);
        
        if (!lines.empty()) ss << std::move(lines[0]);
        
        for (decltype(lines.size()) i = 1; i < lines.size(); ++i)
            ss << endl << std::move(lines[i]);

        training_data = ss.str();
    }

    // Here we find out what the topology is, so we can check it against the size of the wordlist.
    // If they are different, then we have a problem. (Probably  because the wordlist is probably
    // newer than the training data.) Otherwise, everything's OK. (Actually it's not because the
    // wordlist could be different but the same length, but oh well. I have a deadline to meet.)

    static std::regex r(R"end(^topology: ([[:digit:]]+))end");
    std::smatch m;

    // If the training data is in the wrong format, explode.
    if (!std::regex_search(training_data, m, r))
        throw exc::format("Could not load training data: 'topology' line not present", path, 1);

    // Otherwise, since it consists of only digits, it should be safe to convert straight to an int.
    // Of course this will fail if the topology is some obscenely large number, but who cares.
    int topology = std::stoi(m[1]);

    if (topology != _wordlist.size())
        throw exc::format("Training data topology does not match wordlist size."
            " (Probably because the wordlist is newer than the training data; try rebuilding the"
            " training data)", path, 1);

    return training_data;
}

}

}