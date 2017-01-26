#ifndef __MIDI_OUT_HPP
#define __MIDI_OUT_HPP

/***** Includes *****/

#include <cstdint>
#include <alsa/asoundlib.h>


/***** Defines *****/


/***** Classes *****/

class MidiOut {
    
    public:
        MidiOut(
            void);
        
        ~MidiOut(
            void);
        
        int32_t open(
            const char* p_port_name);
        
        int32_t note_on(
            uint8_t note,
            uint8_t velocity);
        
        int32_t note_off(
            uint8_t note,
            uint8_t velocity);
        
    private:
        snd_rawmidi_t* p_handle_;
};


#endif
