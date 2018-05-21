#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "Logger.h"

namespace fakenews
{
  
enum Flags
{
    BOLD   = 0x01,
    STDERR = 0x02
};

// Prints a message in a given colour. See the `Flags` enum. Defined up here and given two bodies
// below for Windows and Linux.
void print_colour(const string& msg, ConsoleColour colour, int flags = 0);

// This provides `isatty()` and `fileno()`.
#if defined _MSC_VER || defined __MINGW32__
#include <Windows.h>
#include <io.h>

const WORD WIN_LOOKUP[] =
{
    0,
    7,
    15,
    4,
    2,
    1,
    3,
    5,
    6,
    12,
    10,
    9,
    11,
    13,
    14
    // There isn't an attribute for DEFAULT...
};

void print_colour(const string& msg, ConsoleColour colour, int flags)
{
    if (flags & STDERR)
    {
        HANDLE console = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(console, &info);
        WORD old_attr = info.wAttributes;

        // There's no way to reset colours to the default, so we just do nothing.
        if (!(colour == DEFAULT || colour == CCOL_N) && _isatty(_fileno(stderr)))
            SetConsoleTextAttribute(console, WIN_LOOKUP[colour]);

        // We also can't do bold, so just ignore that.

        cerr << msg;

        // Reset colours to whatever they were before.
        if (!(colour == DEFAULT || colour == CCOL_N) && _isatty(_fileno(stderr)))
            SetConsoleTextAttribute(console, old_attr);
    }

    else
    {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(console, &info);
        WORD old_attr = info.wAttributes;

        if (!(colour == DEFAULT || colour == CCOL_N) && _isatty(_fileno(stdout)))
            SetConsoleTextAttribute(console, WIN_LOOKUP[colour]);

        cout << msg;

        if (!(colour == DEFAULT || colour == CCOL_N) && _isatty(_fileno(stdout)))
            SetConsoleTextAttribute(console, old_attr);
    }
}
#else
#include <unistd.h>
// This relies on ConsoleColour not changing, which it almost certainly never will.
const char* UNIX_LOOKUP[] =
{
    "\x1B[30m",
    "\x1B[37m",
    "\x1B[97m",
    "\x1B[31m",
    "\x1B[32m",
    "\x1B[34m",
    "\x1B[36m",
    "\x1B[35m",
    "\x1B[33m",
    "\x1B[91m",
    "\x1B[92m",
    "\x1B[94m",
    "\x1B[96m",
    "\x1B[95m",
    "\x1B[93m",
    "\x1B[39m"
};

// Prints a message in a given colour. See the `Flags` enum.
void print_colour(const string& msg, ConsoleColour colour, int flags)
{
    if (flags & STDERR)
    {
        if (isatty(STDERR_FILENO))
        {
            if (flags & BOLD) cerr << "\x1B[1m";
            cerr << UNIX_LOOKUP[colour == CCOL_N ? DEFAULT : colour] << msg << UNIX_LOOKUP[DEFAULT];
            if (flags & BOLD) cerr << "\x1B[0m";
        }

        else cerr << msg;
    }

    else
    {
        if (isatty(STDOUT_FILENO))
        {
            if (flags & BOLD) cout << "\x1B[1m";
            cout << UNIX_LOOKUP[colour == CCOL_N ? DEFAULT : colour] << msg << UNIX_LOOKUP[DEFAULT];
            if (flags & BOLD) cout << "\x1B[0m";
        }

        else cout << msg;
    }
}
#endif

// Makes the '(L123, C321) in 'file.txt':' part of the message.
string info(const string& which = "", size_t line = 0, size_t column = 0)
{
    std::stringstream ss;

    if (line || column || !which.empty()) ss << '(';

    if (line || column)
    {
        if (line)   ss << "L" << setw(3) << setfill('0') << line;
        if (line && column) ss << ", ";
        if (column) ss << "C" << setw(3) << setfill('0') << column;
        ss << ')';

        if (!which.empty()) ss << " in '";
    }

    if (!which.empty()) ss << which;

    if ((line || column) && !which.empty()) ss << '\'';

    if (!(line || column) && !which.empty()) ss << ')';

    ss << ':';
    return ss.str();
}

std::ostream& msg(const string& text, ConsoleColour colour, const string& which, size_t line,
        size_t column, bool err)
// Prints the start part of a message. (e.g. 'Error:')
// text:   The prompt at the start. (So 'error' or 'log' or whatever.)
// colour: The colour that `text` should be printed as.
// which:  Passed to `info()`.
// line:   Passed to `info()`.
// column: Passed to `info()`.
// err:    If true, print to stderr with `cerr`. If false, print to stdout with `cout`.
{
    size_t width = 7 - (text.size() > 7 ? 7 : text.size());
    std::stringstream ss;

    ss << '['
        << setw((width / 2) + (width % 2) + text.size()) << text
        << setw(width / 2 + 1) << ']';

    print_colour(ss.str(), colour, err | BOLD);
    print_colour(info(which, line, column), WHITE, err | BOLD);
    
    std::ostream& output = err ? cerr : cout;
    output << ' ' << flush;
    return output;
}

// Now the implementations of `Logger`.

ostream& Logger::operator()(const string& which, size_t line, size_t column)
{ return msg(_text, _colour, which, line, column, _err); }

ostream& Logger::operator<<(const string& value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(const char* value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(const void* value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(long long value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(unsigned long long value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(double value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(long double value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

ostream& Logger::operator<<(bool value)
{ return msg(_text, _colour, "", 0, 0, _err) << value; }

}
