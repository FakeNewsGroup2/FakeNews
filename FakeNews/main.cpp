#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>

#include "exc.h"

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

vector<string> load_list(const string& path, bool UNIX = false)
// Loads a file, line by line.
// path: The path to a text file to load.
// UNIX: Whether to expect UNIX ('\n') line endings. Otherwise expects DOS ('\r\n'). Defaults to
//       `false`.
// Returns a `vector<string>`, where each element is a line in the file.
{
    vector<string> lines;
    ifstream input(path);
    if (!input) throw exc::file(string("File at '") + path + "' could not be opened for reading.");
    for (string line; getline(input, line, '\n'); lines.emplace_back(std::move(line)));
    if (!UNIX) for (string& s : lines) s.pop_back(); // Remove the trailing '\r,' if needed.
    for (string& s : lines) s.shrink_to_fit();
    lines.shrink_to_fit();
    return lines;
}

int main(int argc, char* argv[])
{
    vector<string> domains;

    for (int i = 0; i < argc; ++i)
    {
        // TODO Eventually, we'll get the domain part from the URL, ensuring it's a valid URL in the
        // process and throwing a child of `exc::exception` if it's not.
        domains.emplace_back(argv[0]);
    }

    vector<string> whitelist;
    vector<string> blacklist;

    try
    {
        whitelist = load_list("whitelist.txt");
        blacklist = load_list("blacklist.txt");
    }

    catch (const exc::exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    cout << "WHITELIST:" << endl;
    for (const string& s : whitelist) cout << s << endl;

    cout << "\nBLACKLIST:" << endl;
    for (const string& s : blacklist) cout << s << endl;

    // TODO Do stuff!

    return 0;
}