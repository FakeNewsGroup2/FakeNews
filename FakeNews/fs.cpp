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

// All this does is wraps strerror, so that it compiles with both MSVC and GCC.
// Behaves like GNU strerror_r.
char* error(char* buf, size_t len);

#ifdef _MSC_VER
char* error(char* buf, size_t len)
{
    strerror_s(buf, len, errno);
    return buf;
}
#else
char* error(char* buf, size_t len)
{
    return strerror_r(errno, buf, len);
}
#endif

namespace fs
{

vector<string> load_lines(const string& path, bool UNIX)
{
    vector<string> lines;
    std::ifstream input(path);

    if (!input)
    {
        std::stringstream ss;
        char err[256];
        ss << "File at '" << path << "' could not be opened for reading: "
            << error(err, sizeof(err));
        throw exc::file(ss.str());
    }

    for (string line; getline(input, line, '\n'); lines.push_back(std::move(line)));
    if (!UNIX) for (string& s : lines) s.pop_back(); // Remove the trailing '\r,' if needed.
    for (string& s : lines) s.shrink_to_fit();
    lines.shrink_to_fit();
    return lines;
}

}

}
