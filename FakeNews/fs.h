#pragma once

#include <vector>
#include <string>

// This file contains items relating to interactions with the file system.

namespace fakenews
{

namespace fs
{

// All this does is wraps strerror, so that it compiles with both MSVC, GCC, and MinGW.
// Behaves like GNU strerror_r.
char* error(char* buf, size_t len);

// Loads a file, line by line.
// path: The path to a text file to load.
// UNIX: Whether to expect UNIX ('\n') line endings. Otherwise expects DOS ('\r\n'). Defaults to
//       `false`.
// Returns a `vector<string>`, where each element is a line in the file.
// Throws `exc::file` if the file at `path` could not be opened for reading.
std::vector<std::string> load_lines(const std::string& path, bool UNIX = false);

}

}
