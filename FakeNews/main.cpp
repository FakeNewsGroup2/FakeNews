#include <iostream>


/*
Group project - Fake news assessor.
This program accepts URL strings as inputs, 
and determines if the given webpage is likely fake or a real news source.

Members: 
16609509 - James Coe
16606590 - Paulius Vaitaitis
16606229 - Lee Milner
15595025 - Ethan Ansell
16641828 - Jiahe Wang
15625064 - Dahai Zhu
16609305 - Ashley Worth

*/

using namespace std;

class InputClass
{
public:
	string URL;
};

class Lists
{
public:
	string Trusted[1] = {};
	string Fake[];
};

int main()
{
	cout << "Enter URL of webpage/news article to assess:" << endl;
	InputClass.URL = cin.get();
// Compare URL to list strings - server string only

}
