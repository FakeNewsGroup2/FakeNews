#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <string.h>

#include "fs.h"
#include "exc.h"

using std::vector;
using std::string;

namespace fakenews
{

namespace fs
{

#if defined _MSC_VER || defined __MINGW32__
#include <Windows.h>
string error()
{
    char buf[256];
    strerror_s(buf, sizeof(buf), errno);
    return string(buf);
}

vector<string> get_files(const string& path)
{
    // Windows version only supports ASCII, screw it.

    string glob_path(path); // This version is modified to end in '\*'.
    
    // If it's empty, quit.
    // If it ends in '\' remove it.
    // Now make sure it exists, and is a directory.
    // Add '\*'.
    
    if (glob_path.empty()) throw exc::file("Path is empty");
    if (glob_path[glob_path.size() - 1] == '\\') glob_path.pop_back();

    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA(glob_path.c_str(), &data);
    
    if (find == INVALID_HANDLE_VALUE) throw exc::file("Could not read directory", path);
    if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        FindClose(find);
        throw exc::file("Not a directory", path);
    }

    glob_path += "\\*";
    string trail_path(path); // This version ends with '\'.
    if (trail_path[trail_path.size() - 1] != '\\') trail_path.push_back('\\');

    FindClose(find);
    vector<string> result;

    for (find = FindFirstFileA(glob_path.c_str(), &data); FindNextFileA(find, &data);)
    {
        if (find == INVALID_HANDLE_VALUE) throw exc::file("Could not read directory", path);
        
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            result.emplace_back(trail_path + data.cFileName);
    }

    FindClose(find);
    return result;
}
#else
char* error(char* buf, size_t len)
{
    return strerror_r(errno, buf, len);
}

string error()
{
    char buf[256];
    return string(strerror_r(errno, buf, sizeof(buf)));
}

// TODO implement `get_files()` for Linux
#endif

vector<string> load_lines(const string& path)
{
    vector<string> lines;
    std::ifstream input(path);

    if (!input) throw exc::file(error(), path);

    for (string line; getline(input, line, '\n'); lines.push_back(std::move(line)));
    for (string& s : lines) s.shrink_to_fit();
    lines.shrink_to_fit();
    return lines;
}

}

}
