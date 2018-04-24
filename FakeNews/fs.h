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

// Loads a file, line by line. Because this calls `std::getline()`, it depends on the platform which
// line endings it expects.
// path: The path to a text file to load.
// Returns a `vector<string>`, where each element is a line in the file.
// Throws `exc::file` if the file at `path` could not be opened for reading.
std::vector<std::string> load_lines(const std::string& path);

// Gets a list of every file in a directory. 'File' only means regular files. Not recursive.
// path: The path to the directory to list.
// Returns a `vector<string>`, where each element is a full, absolute path to a file.
// Throws `exc::file` if the directory at `path` could not be read.
std::vector<std::string> get_files(const std::string& path);

}

}
