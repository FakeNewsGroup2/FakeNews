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

Article::Article(const string& path, ArticleVeracity veracity):
    _address(nullptr),
    _veracity(veracity)
{
    vector<string> lines = fs::load_lines(path);
    
    if (lines.size() < 3) throw exc::format("Not enough lines", path);
    
    // If the URL was the wrong format, rethrow the exception with some extra info.
    try                          { _address = new net::Address(lines[0]); }
    catch (const exc::format& e) { throw exc::format(e.what(), path, 1);  }

    _headline = lines[1];
    
    for (decltype(lines)::size_type i = 2; i < lines.size(); ++i)
    {
        _contents += lines[i];
        _contents += '\n';
    }
}

}

}