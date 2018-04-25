#pragma once

// This file contains misc small utility functions.

#include <string>

namespace fakenews
{

namespace util
{

// Converts a string to upper case. Uses `toupper()`.
// s: The string to convert.
// Returns a copy of `s`, converted to upper case.
std::string upper(const std::string& s);

}

}