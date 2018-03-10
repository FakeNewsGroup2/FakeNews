#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>

#include <cstdlib>

#include "curl/curl.h"

#include "fs.h"
#include "exc.h"
#include "net.h"

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

int main(int argc, char* argv[])
{
    vector<net::Address> addresses;
    vector<string> whitelist;
    vector<string> blacklist;

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

    try
    {
        whitelist = fs::load_lines("whitelist.txt");
	    blacklist = fs::load_lines("blacklist.txt");
    }

    catch (const exc::exception& e) { die(e.what()); }

    // TODO Do stuff!

    // Use the new Address class to split a URL into its parts!
    cout << "URLs:" << endl;
    for (net::Address& a : addresses)
    {
        cout << a.full() << endl;
        cout << '\t' << "protocol: " << (a.protocol().empty() ? "<none>" : a.protocol()) << endl;
        cout << '\t' << "resource: " << a.resource() << endl;
        cout << '\t' << "request:  " << (a.request().empty() ? "<none>" : a.request()) << endl;
    }

    // Try downloading an internet page with `get_file()`.
    string url = ("http://google.com");
    cout << "Fetching '" << url << "'..." << endl;
    string google = net::get_file(url);

    // Print out the first 25 characters. (Or all of them if there's less than 25.)
    string::size_type size = 25 < google.size() ? 25 : google.size();
    for (int i = 0; i < size; ++i) cout << google[i];
    cout << (size == 25 ? "..." : "") << endl; // If there was more than 25 characters, print '...'

    cleanup();
    // system("pause"); // Please no.... just press Ctrl-F5 to run the program instead.
                        // (Or run it with command prompt as nature intended...)
    return 0;
}
