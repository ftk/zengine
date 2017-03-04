//
// Created by fotyev on 2017-03-01.
//

#include "log.hpp"

#include <iostream>

#include <ctime>

namespace log_detail {
std::vector<logger> loggers{{std::cerr, log_level::debug2, 0}};




void log_start(const logger& l)
{
    if(l.flags & TIME)
    {
        time_t stamp = time(nullptr);
        struct tm * timeinfo = localtime(&stamp);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "[%H:%M:%S] ", timeinfo);
        l.s << buffer;
    }
}

void log_end(const logger& l)
{
    // end of line
    if(!(l.flags & NO_NEWLINE))
        l.s << '\n';
    if(!(l.flags & NO_FLUSH))
        l.s.flush();
}

} //namespace log_detail

