#pragma once

#include <vector>
#include <string>

// This file contains items relating to interactions with the file system.

namespace fakenews
{

namespace fs
{

// All this does is wraps strerror, so that it compiles with MSVC, GCC, and MinGW.
// Gets a string describing the last file system error. Uses strerror_s (or GNU strerror_r,
// depending) underneath.
std::string error();

// Loads a file, line by line. Because this calls `std::getline()`, it depends on the platform which
// line endings it expects.
// path: The path to a text file to load.
// Returns a `vector<string>`, where each element is a line in the file.
// Throws `exc::file` if the file at `path` could not be opened for reading.
// Throws `exc::arg` if `path` is empty.
std::vector<std::string> load_lines(const std::string& path);

// Gets a list of every file in a directory. 'File' only means regular files. Not recursive.
// path: The path to the directory to list.
// Returns a `vector<string>`, where each element is a full, absolute path to a file.
// Throws `exc::file` if the directory at `path` could not be read.
std::vector<std::string> get_files(const std::string& path);

// Reads the entire contents of a file to a string.
// path: The path to the file to load.
// Returns a string with the contents of the file.
// Throws `exc::file` if the file at `path` could not be opened for reading.
// Throws `exc::arg` if `path` is empy.
std::string read_to_string(const std::string& path);

}

}
