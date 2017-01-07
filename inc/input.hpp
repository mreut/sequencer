#ifndef __INPUT_HPP
#define __INPUT_HPP

/***** Includes *****/

#include <cstdint>


/***** Defines *****/

#define INPUT_COMMAND_INVALID 0
#define INPUT_COMMAND_EXIT 1
#define INPUT_COMMAND_SET_NOTE 2
#define INPUT_COMMAND_CLEAR_NOTE 3
#define INPUT_COMMAND_SET_INDEX 4
#define INPUT_COMMAND_SET_BPM 5
#define INPUT_COMMAND_START_PLAY 6
#define INPUT_COMMAND_STOP_PLAY 7


/***** Global Functions *****/

extern int32_t input_start(
    void);

extern int32_t input_stop(
    void);

extern int32_t input_get(
    uint8_t& command,
    uint8_t& data);

#endif
