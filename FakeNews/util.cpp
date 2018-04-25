#include "util.h"

#include <string>

using std::string;

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

}

}