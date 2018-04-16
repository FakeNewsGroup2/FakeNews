#include "log.h"

#include "Logger.h"

namespace fakenews
{

namespace log
{

    
// Light blue is hard to read on windows cmd, so have that be light cyan instead.
#if defined _MSC_VER || defined __MINGW32__
Logger log("Log", LIGHT_CYAN);
#else
Logger log("Log", LIGHT_BLUE);
#endif

Logger error("Error", LIGHT_RED);
Logger warning("Warning", LIGHT_YELLOW);
Logger success("Success", GREEN);

}

}