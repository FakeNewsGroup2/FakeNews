#include "util.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>

#include "fs.h"
#include "log.h"
#include "exc.h"

using std::endl;
using std::string;
using std::vector;

namespace fakenews
{

namespace util
{

const std::regex REGEX_WORD("[A-Za-z\\-']+");
const std::regex REGEX_WHITESPACE("[^\\s]+");

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

vector<string> load_clean_warn(const string& path, const string& contents, bool case_s)
{
    vector<string> lines = fs::load_lines(path);
    for (string& line : lines) util::trim(line);

    // Remove duplicates/blank entries. Since we called `util::trim()` on each line, lines which
    // consisted of just whitespace will also be removed.
    vector<string*> duplicates = util::cleanup(lines, case_s);

    if (!duplicates.empty())
    {
        // Only print up to this many duplicates in the warning message. (Don't want to spam them.)
        decltype(duplicates.size()) max_to_print = 10;

        std::stringstream ss;
        
        ss << "File contains duplicate " << contents << ": '" << *duplicates[0] << "'";

        for (decltype(duplicates.size()) i = 1; i < min(duplicates.size(), max_to_print); ++i)
            ss << ", '" << *duplicates[i] << "'";

        if (duplicates.size() > max_to_print)
            ss << " (and " << (duplicates.size() - max_to_print) << " more)";

        log::warning(path) << ss.str() << endl;
    }

    return lines;
}

string::size_type occurrences(const string& haystack, const string& needle)
{
    string::size_type result = 0, pos = 0;

    while ((pos = haystack.find(needle, pos)) != std::string::npos)
    {
        ++result;
        pos += needle.size();
    }

    return result;
}

bool starts_with(const string& s, const string& prefix)
{
    return prefix.size() <= s.size() && s.compare(0, prefix.size(), prefix) == 0;
}

vector<string> load_words(const string& path, const string& contents, bool case_s)
{
    vector<string> words = load_clean_warn(path, contents, case_s);

    for (string& word : words)
    {
        trim(word);

        for (char c : word)
        {
            // This is silly because it only works for English, but who cares. Sorry everybody who
            // doesn't speak English. You're probably used to computers hating you by now.
            
            // This could have a line number, if I wasn't so lazy.
            if (!std::regex_match(word, std::smatch(), REGEX_WORD))
            {
                stringstream ss;
                ss << "Word '" << word << "' contains illegal characters";
                throw exc::format(ss.str(), path);
            }
        }
    }

    return words;
}

string::size_type count_words(const std::string& s)
{
    return distance(sregex_iterator(s.cbegin(), s.cend(), REGEX_WHITESPACE), sregex_iterator());
}

vector<string> split_words(const std::string& s)
{
    vector<string> result;

    for (auto it = sregex_iterator(s.cbegin(), s.cend(), REGEX_WORD); it != sregex_iterator(); ++it)
        result.emplace_back(smatch(*it)[0]);

    return result;
}

vector<string> split_whitespace(const std::string& s)
{
    vector<string> result;

    for
    (
        auto it = sregex_iterator(s.cbegin(), s.cend(), REGEX_WHITESPACE);
        it != sregex_iterator();
        ++it
    )
        result.emplace_back(smatch(*it)[0]);

    return result;
}

}

}