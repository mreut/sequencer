#ifndef __MIDI_SCORE_HPP
#define __MIDI_SCORE_HPP

/***** Includes *****/

#include <cstdint>


/***** Defines *****/

#define MIDI_SCORE_LENGTH 1024

#define MIDI_NOTE_REST 0xFF

#define MIDI_NOTE_MAX 0x7F


/***** Structs *****/

struct score_step {
    uint8_t note;
    bool is_set;
};


/***** Classes *****/

class MidiScore {
    public:
        MidiScore(
            void);
        
        int32_t set_note(
            uint32_t index,
            uint8_t note);
            
        int32_t get_note(
            uint32_t index,
            uint8_t* p_note);
            
        int32_t clear_note(
            uint32_t index);
            
        bool is_end(
            uint32_t index);
        
    private:
        struct score_step score_[MIDI_SCORE_LENGTH];
        
        int32_t last_note_;
};

#endif
