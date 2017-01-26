#ifndef __UTILITY_HPP
#define __UTILITY_HPP

/***** Includes *****/

#include <cstdint>
#include <string>


/***** Namespace *****/

using namespace std;


/***** Global Functions *****/

extern int32_t delay_ns(
    uint64_t num_nanosecs);

extern bool is_number(
    string& s);

#endif
