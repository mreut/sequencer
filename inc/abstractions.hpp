#ifndef __ABSTRACTIONS_HPP
#define __ABSTRACTIONS_HPP

/***** Includes *****/

#include <cstdint>


/***** Defines *****/

#define MIDI_NOTE_MAX 0x7F


/***** Global Functions *****/

extern int32_t delay_ns(
    uint32_t num_nanosecs);

extern int32_t ascii_to_note(
    uint8_t* p_string,
    uint8_t* p_note);

extern int32_t note_to_ascii(
    uint8_t note,
    uint8_t* p_string);

#endif
