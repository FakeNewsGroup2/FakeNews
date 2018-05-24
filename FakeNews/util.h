#pragma once

// This file contains misc small utility functions.

#include <string>
#include <vector>
#include <algorithm>

#include <cstdlib>

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

// Loads a file line-by-line into a vector. Every line in the file is an element in the final
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

// Counts the number of occurrences of one string within another.
// haystack: The string to look within.
// needle:   The string to look for.
// Returns the number of occurrences.
std::string::size_type occurrences(const std::string& haystack, const std::string& needle);

// Checks whether a string starts with a given prefix or not. Why the hell isn't this in the
// standard library?
// s:      The string to check.
// prefix: The prefix to check for.
// Returns true if `s` starts with `prefix`, false otherwise.
bool starts_with(const std::string& s, const std::string& prefix);

// A wrapper for `load_clean_warn()`, loading a file line-by-line into a vector, but strips
// leading/trailing whitespace from each line, and makes sure that each line contains no spaces or
// punctuation. (Hyphens are allowed.)
// For the parameters, see `load_clean_warn()`.
// Throws `exc::format` if any of the words contain spaces or punctuation.
// Throws anything `load_clean_warn()` throws.
std::vector<std::string> load_words(const std::string& path, const std::string& contents = "words",
    bool case_s = false);

// Counts the number of words in a string. A 'word' is just any sequence of non-whitespace
// characters surrounded by whitespace characters.
// s: The string to count the words of.
// Returns the number of words in `s`.
std::string::size_type count_words(const std::string& s);

// Splits a string into a vector of words. A 'word' is any sequence of alphanumeric characters (or
// hyphens/apostrophes) surrounded by whitespace characters.
// s: The string to split.
// Returns a vector of all the words in `s`.
std::vector<std::string> split_words(const std::string& s);

// Splits a string into regions delimited by whitespace. (Same as `split_words()`, but uses the
// simpler definition of 'word' from `count_words()` instead.)
// s: The string to split.
// Returns a vector of all the non-whitespace regions in `s`.
std::vector<std::string> split_whitespace(const std::string& s);

// Shuffles the elements in a vector into a random order, using C's `rand()` function seeded with
// the current time.
// v: The vector to shuffle.
template<typename T> void shuffle(std::vector<T>& v)
{
    // Unfortunately, you can't implement templates in a separate file.
    std::srand((unsigned int)std::time(NULL));
    for (T& t : v) std::swap(t, v[std::rand() % v.size()]);
}

// Prints the contents of a vector in the format specified below.
// label: The label to print before the contents of the vector. (See below for clarification.)
// vec:   The vector to print.
// Prints in this format:
// Vector of Numbers: 1 2 3 4 5
// ^---- label ----^  ^- vec -^
template<typename T> std::string display_vector(const std::string& label, const std::vector<T>& vec)
{
    std::stringstream ss;
    ss << label << ':';
    for (const T& v : vec) ss << ' ' << v;
    ss << std::endl;
    return ss.str();
}

}

}