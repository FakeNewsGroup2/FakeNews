#pragma once

// This file contains misc small utility functions.

#include <string>
#include <vector>

namespace fakenews
{

namespace util
{

// Converts a string to upper case. Uses `toupper()`.
// s: The string to convert.
// Returns a copy of `s`, converted to upper case.
std::string upper(const std::string& s);

// Removes any leading/trailing whitespace (according to `isspace()` from a string.)
// s: The string to trim.
void trim(std::string& s);

// Goes through every string in a vector, removing any empty/duplicate strings.
// v:      The vector to clean.
// case_s: Whether to check for duplicates case-sensitively. Defaults to false.
// Returns a vector of pointers to strings within `v`, which had duplicates before this function was
// called.
// For each string with duplicates, the first one in the vector is kept, and all subsequent ones
// are removed. No individual element is modified.
// This function invalidates any iterators you have within `v` which point past any elements that
// were removed. Since you don't know where those were, consider them invalid unless `v` didn't
// change.
std::vector<std::string*> cleanup(std::vector<std::string>& v, bool case_s = false);

// Loads a file line by line into a vector. Every line in the file is an element in the final
// vector. Every line has leading/trailing whitespace removed, blank lines are removed
// (including those consisting of only whitespace), and duplicate lines are removed. This
// function prints a warning containing any duplicate lines that were found (up to a certain
// number to avoid screen spam.) Because this calls `fs::load_lines()` which in turn calls
// `std::getline()`, it depends on the platform which line endings it expects.
// path:     The path to the file to load.
// contents: What the file contains, to be printed in the warning message. (e.g. 'file contains
//           duplicate lines.') Defaults to 'lines.'
// case_s:   Whether to search for duplicates case-sensitively. Defaults to false.
// Throws anything `fs::load_lines()` throws.
std::vector<std::string> load_clean_warn(const std::string& path,
    const std::string& contents = "lines", bool case_s = false);

}

}