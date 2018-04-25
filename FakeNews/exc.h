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
    // what:  String describing the error.
    // which: The item that failed (e.g. file path, website URL.)
    exception(const std::string& what, const std::string& which = ""): _what(what), _which(which)
        { }

    const std::string& what() const  { return _what;  }
    const std::string& which() const { return _which; }

    protected:
    std::string _what;
    std::string _which;
};

class unimplemented: public exception
// To throw when something is not yet implemented. This should act as a placeholder while building
// the program and should not be thrown in any release version.
{
    public:
    // Prepends 'UNIMPLEMENTED: ' to `what` so that messages are clearer when printed.
    unimplemented(const std::string& what, const std::string& which = ""):
        exception(std::string("UNIMPLEMENTED: ") + what, which)
    { }
};

class file: public exception
// To throw when some kind of file operation has failed.
{
    public:
    file(const std::string& what, const std::string& which = ""): exception(what, which) { }
};

class format: public exception
// To throw when something isn't in a valid format, e.g. a URL, or a file.
{
    public:
    // line:   The line in the file where the problem occurs, starting at 1, 0 if N/A.
    // column: The column in the file where the problem occurs, starting at 1, 0 if N/A.
    // `line` and `column` are only logically applicable if `which` is a file.
    format(const std::string& what, const std::string& which = "", size_t line = 0,
        size_t column = 0):
        exception(what, which),
        _line(line),
        _column(column)
    { }

    size_t line()             const { return _line;   }
    size_t column()           const { return _column; }

    private:
    size_t _line;
    size_t _column;
};

class init: public exception
// To throw when there was a problem initialising the program or some library.
{
    public:
    init(const std::string& what, const std::string& which = ""): exception(what, which) { }
};

class net: public exception
// To throw when there is some problem involving networking, such as a connection error.
{
    public:
    net(const std::string& what, const std::string& which = ""): exception(what, which) { }
};

class arg: public exception
// To throw when some idiot doesn't read the documentation properly, and passes the wrong value into
// a function. Use extremely sparingly. ('sqrt(-1)' or 'div(1, 0)' for example.)
{
    public:
    arg(const std::string& what, const std::string& which = ""): exception(what, which) { }
};

}

}