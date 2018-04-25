#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

namespace fakenews
{

// TODO Don't let this be accessible from outside.
enum ConsoleColour
{
    BLACK,
    GREY,
    WHITE,

    RED,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW,

    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_BLUE,
    LIGHT_CYAN,
    LIGHT_MAGENTA,
    LIGHT_YELLOW,

    DEFAULT,

    CCOL_N
};

class Logger
{
    public:
    Logger(const string& text, ConsoleColour colour, bool err = false):
        _text(text),
        _colour(colour),
        _err(err)
    { }

    ostream& operator()(const string& which = "", size_t line = 0, size_t column = 0);

    // We wrap wrap all these... there's got to be a better way...
    ostream& operator<<(const string& value);
    ostream& operator<<(const char* value);
    ostream& operator<<(const void* value);
    ostream& operator<<(long long value);
    ostream& operator<<(unsigned long long value);
    ostream& operator<<(double value);
    ostream& operator<<(long double value);
    ostream& operator<<(bool value);

    private:
    string _text;
    ConsoleColour _colour;
    bool _err;
};

}