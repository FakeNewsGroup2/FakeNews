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

class InputURL
{
 public:
	 InputURL()
	 {}
	 ~InputURL()
	 {}
	 void SetUrl(string str)
	 {
		 URL = str;
	 }

private:
	string URL;
};

class Lists
{
public:
	
	Lists()
	{}
	~Lists()
	{}
	//simply set whitelist
	void SetWhitelist(vector<string> str)
	{
		whitelist = str;
	}
	//push something onto end of whitelist
	void AddStringWhitelist(string str)
	{
		whitelist.push_back(str);
	}
	//returns contents of whitelist
	std::vector<string> GetCopyOfWhitelist()
	{
		return whitelist;
	}
	//slightly more detailed contents, with line nums
	void WhitelistVectorContents()
	{
		for (unsigned int i = 0; i < whitelist.size(); i++)
		{
			std::cout << "Element[" << i << "] = " << whitelist[i] << std::endl;
		}
		std::cout << std::endl;
	}
	//simply set blacklist
	void SetBlacklist(vector<string> str)
	{
		blacklist = str;
	}
	//append something onto end
	void AddStringBlacklist(string str)
	{
		blacklist.push_back(str);
	}
	//returns contents of blacklist
	std::vector<string> GetCopyOfBlacklist()
	{
		return blacklist;
	}
	//as above
	void BlacklistVectorContents()
	{
		for (unsigned int i = 0; i < blacklist.size(); i++)
		{
			std::cout << "Element[" << i << "] = " << blacklist[i] << std::endl;
		}
		std::cout << std::endl;
	}
	void printWhite()
	{
		cout << "WHITELIST:" << endl;
		for (const string& s : whitelist) cout << s << endl;
	}
	void printBlack()
	{
		cout << "\nBLACKLIST:" << endl;
		for (const string& s : blacklist) cout << s << endl;
	}

private:
	vector<string> whitelist;
	vector<string> blacklist;

};


int main(int argc, char* argv[])
{
    vector<string> domains;
    
    InputURL input;
	Lists lists;

    for (int i = 0; i < argc; ++i)
    {
        // TODO Eventually, we'll get the domain part from the URL, ensuring it's a valid URL in the
        // process and throwing a child of `exc::exception` if it's not.
        domains.emplace_back(argv[0]);
    }

    cout << "Enter URL of webpage/news article to assess:" << endl;
	string cintmp;
	std::cin >> cintmp;

	input.SetUrl(cintmp);

    try
    {
        lists.SetBlacklist(load_list("whitelist.txt"));
		lists.SetWhitelist(load_list("blacklist.txt"));
    }

    catch (const exc::exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    lists.printWhite();
    

	lists.printBlack();

    // TODO Do stuff!
    system("pause");
    return 0;
}
