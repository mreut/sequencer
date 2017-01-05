#ifndef __ABSTRACTIONS_HPP
#define __ABSTRACTIONS_HPP

/***** Includes *****/

#include <cstdint>
#include <string>


/***** Namespace *****/

using namespace std;


/***** Defines *****/

#define MIDI_NOTE_MAX 0x7F


/***** Global Functions *****/

extern int32_t delay_ns(
    uint32_t num_nanosecs);

extern int32_t ascii_to_note(
    string& ascii,
    uint8_t& note);

extern int32_t note_to_ascii(
    uint8_t note,
    string& ascii);

#endif
