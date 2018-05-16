#include "util.h"

#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;

namespace fakenews
{

namespace util
{

string upper(const string& s)
{
    string result;
    result.reserve(s.size());
    for (const char& c : s) result += toupper(c);
    result.shrink_to_fit(); // Probably pointless.
    return result;
}

void trim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !isspace(ch); }).base(),
        s.end());
}

vector<string*> cleanup(vector<string>& v, bool case_s)
{
    vector<string*> duplicates;

    // Remove blank entries.
    v.erase
    (
        std::remove_if(v.begin(), v.end(), [](const string& t) -> bool { return t.empty(); }),
        v.end()
    );

    // Remove duplicates.
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        vector<string>::iterator new_end = case_s
            
            ? std::remove_if(it + 1, v.end(),
                [it](const string& t) -> bool { return t == *it; })

            : std::remove_if(it + 1, v.end(),
                [it](const string& t) -> bool { return util::upper(t) == util::upper(*it); });

        if (new_end != v.end())
        {
            duplicates.push_back(&(*it));
            v.erase(new_end, v.end());
        }
    }

    return duplicates;
}

}

}