#pragma once
//This header file contains class files pertaining to whitelists and blacklists.
using std::cout;
using std::endl;
using std::string;
using std::vector;


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
