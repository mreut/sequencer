#ifndef __MIDI_SCORE_HPP
#define __MIDI_SCORE_HPP

/***** Includes *****/

#include <cstdint>
#include <mutex>
#include <string>


/***** Defines *****/

#define MIDI_SCORE_LENGTH 1024

#define MIDI_NOTE_REST 0xFF

#define MIDI_NOTE_MAX 0x7F


/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum count_type {
    COUNT_MULTIPLY = 0,
    COUNT_DIVIDE = 1,
};


/***** Structs *****/

struct score_step {
    uint8_t note;
    uint32_t count;
    enum count_type type;
};


/***** Classes *****/

class MidiScore {
    public:
        MidiScore(
            void);
        
        int32_t save(
            string name);
            
        int32_t load(
            string name);
        
        int32_t set_bpm(
            uint16_t bpm);
        
        uint16_t get_bpm(
            void);
        
        int32_t set_repeat(
            uint8_t repeat);
        
        uint8_t get_repeat(
            void);
        
        int32_t set_note(
            uint32_t index,
            uint8_t note);
            
        uint8_t get_note(
            uint32_t index);
        
        int32_t set_count(
            uint32_t index,
            uint8_t count);
            
        uint8_t get_count(
            uint32_t index);
        
        int32_t set_count_type(
            uint32_t index,
            enum count_type type);
            
        enum count_type get_count_type(
            uint32_t index);
        
        int32_t set_note_count(
                    uint32_t index,
                    uint8_t note,
                    enum count_type type,
                    uint8_t count);
        
        int32_t get_note_count(
            uint32_t index,
            uint8_t& note,
            enum count_type& type,
            uint8_t& count);
            
        int32_t clear_note(
            uint32_t index);
            
        bool is_end(
            uint32_t index);
        
    private:
        struct score_step score_[MIDI_SCORE_LENGTH];
        
        uint16_t bpm_;
        
        uint8_t repeat_;
        
        int32_t last_note_;
        
        mutex mutex_;
};


/***** Global Functions *****/

extern bool ascii_to_note(
    string& ascii,
    uint8_t& note);

extern bool ascii_to_note_count(
    string& ascii,
    uint8_t& note,
    enum count_type& type,
    uint8_t& count);

extern bool note_to_ascii(
    uint8_t note,
    string& ascii);

extern bool note_count_to_ascii(
    uint8_t note,
    enum count_type type,
    uint8_t count,
    string& ascii);

#endif
