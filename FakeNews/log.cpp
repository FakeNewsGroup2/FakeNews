#include "log.h"

#include "Logger.h"

namespace fakenews
{

namespace log
{
// These all print to stderr so that the actual output of the program can be separated from log
// messages.
    
// Light blue is hard to read on windows cmd, so have that be light cyan instead.
#if defined _MSC_VER || defined __MINGW32__
Logger log("Log", LIGHT_CYAN, true);
#else
Logger log("Log", LIGHT_BLUE, true);
#endif

Logger error("Error", LIGHT_RED, true);
Logger warning("Warning", LIGHT_YELLOW, true);
Logger success("Success", GREEN, true);

}

}