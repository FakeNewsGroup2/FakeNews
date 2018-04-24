#include <iostream>
#include <vector>
#include <string>

#include "Article.h"
#include "fs.h"
#include "exc.h"

using std::string;
using std::vector;

namespace fakenews
{

namespace article

{

Article::Article(const string& path): _address(nullptr)
{
    vector<string> lines = fs::load_lines(path);
    
    if (lines.size() < 3) throw exc::format("Not enough lines");
    
    _address = new net::Address(lines[0]);
    _headline = lines[1];
    
    for (decltype(lines)::size_type i = 2; i < lines.size(); ++i)
    {
        _contents += lines[i];
        _contents += '\n';
    }
}

}

}