#pragma once

// This file contains various exceptions for use within this program.

// #################################################################################################
// # NOTE: A VERSION OF THIS FILE HAS BEEN USED IN A PREVIOUS ASSIGNMENT CMP2090M-1718 -           #
// #       OBJECT-ORIENTED PROGRAMMING ITEM 1, BY ETHAN ANSELL ANS15595025.                        #
// #       IT HAS HAD MANY CLASSES ADDED/REMOVED.                                                  #
// #################################################################################################

#include <string>

namespace fakenews
{

namespace exc
{

class exception
// An abstract exception type. Any exception thrown by us should derive from this. Any exceptions
// which make it to `main()` uncaught, and do not derive from this, are a bug. This exception should
// NOT be thrown, only caught.
{
    public:
    exception(const std::string& what) : _what(what) { }
    const std::string& what() const { return _what; }

    protected:
    std::string _what;
};

class unimplemented: public exception
// To throw when something is not yet implemented. This should act as a placeholder while building
// the program and should not be thrown in any release version.
{
    public:
    // Prepends 'UNIMPLEMENTED: ' to `what` so that messages are clearer when printed.
    unimplemented(const std::string& what): exception(std::string("UNIMPLEMENTED: ") + what) { }
};

class file: public exception
// To throw when some kind of file operation has failed.
{
    public:
    file(const std::string& what, const std::string& path = ""): exception(what), _path(path) { }
    const std::string& path() const { return _path; }

    private:
    std::string _path;
};

class format: public exception
// To throw when something isn't in a valid format, e.g. a URL, or a file.
{
    public:
    // path:   The path to the file, if any.
    // line:   The line in the file where the problem occurs, starting at 1, 0 if N/A.
    // column: The column in the file where the problem occurs, starting at 1, 0 if N/A.
    format(const std::string& what, const std::string& path = "", size_t line = 0,
        size_t column = 0):
        exception(what),
        _path(path),
        _line(line),
        _column(column)
    { }

    const std::string& path() const { return _path;   }
    size_t line()             const { return _line;   }
    size_t column()           const { return _column; }

    private:
    std::string _path;
    size_t _line;
    size_t _column;
};

class init: public exception
// To throw when there was a problem initialising the program or some library.
{
    public:
    init(const std::string& what): exception(what) { }
};

class net: public exception
// To throw when there is some problem involving networking, such as a connection error.
{
    public:
    net(const std::string& what): exception(what) { }
};

}

}