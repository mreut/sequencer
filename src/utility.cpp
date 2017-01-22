/***** Includes *****/

#include <algorithm> 
#include <cctype>
#include <ctime>
#include <cerrno>

#include "utility.hpp"


/***** Global Functions *****/

int32_t delay_ns(
    uint64_t num_nanosecs)
{
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    
    while (num_nanosecs >= 1e9) {
        num_nanosecs -= 1e9;
        ts.tv_sec += 1;
    }
    ts.tv_nsec = num_nanosecs;
    
    while (nanosleep(&ts, &ts) == -1) {
        if ( (errno == ENOSYS) || (errno == EINVAL)) {
            fprintf(stderr, "Error: nanosleep failed\r\n");
            return -1;
        }
    }
    
    return 0;
}

bool is_number(
    string& s)
{
    return !s.empty() && 
           find_if(s.begin(),
                   s.end(),
                   [](char c) { return !isdigit(c); }) == s.end();
}
