/***** Inlcudes *****/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include "abstractions.hpp"
#include "MidiScore.hpp"

/***** Global Functions *****/

int32_t delay_ns(
    uint32_t num_nanosecs)
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

int32_t ascii_to_note(
    string& ascii,
    uint8_t& note)
{
    uint8_t tmp = 0;
    uint8_t octave = 0;
    
    if ('-' == ascii[0]) {
        note = MIDI_NOTE_REST;
    }
    else {
        switch(ascii[0]) {
        case ('A'):
            tmp = 9;
            break;
        case ('B'):
            tmp = 11;
            break;
        case ('C'):
            tmp = 0;
            break;
        case ('D'):
            tmp = 2;
            break;
        case ('E'):
            tmp = 4;
            break;
        case ('F'):
            tmp = 5;
            break;
        case ('G'):
            tmp = 7;
            break;
        default:
            return -1;
        }
        
        if (isdigit(ascii[1])) {
            if (isdigit(ascii[2])) {
                octave = 10;
            }
            else {
                octave = ascii[1] - '0';
            }
        }
        else {
            return -1;
        }
        
        tmp += 12 * octave;
        
        if (('#' == ascii[2]) || ('#' == ascii[3])) {
            tmp += 1;
        }
        
        if (tmp > MIDI_NOTE_MAX) {
            return -1;
        }
        
        note = tmp;
    }
    
    return 0;
}

int32_t note_to_ascii(
    uint8_t note,
    string& ascii)
{
    int32_t octave = 0;
    int32_t r = -1;
    bool is_sharp = false;

    if (note == MIDI_NOTE_REST) {
        ascii = '-';
        r = 0;
    }
    else if (note < MIDI_NOTE_MAX) {    
        while (note >= 12) {
            octave++;
            note -= 12;
        }
        
        switch (note) {
        case (0):
            note = 'C';
            break;
            
        case (1):
            note = 'C';
            is_sharp = true;
            break;
        
        case (2):
            note = 'D';
            break;
            
        case (3):
            note = 'D';
            is_sharp = true;
            break;
            
        case (4):
            note = 'E';
            break;
            
        case (5):
            note = 'F';
            break;
            
        case (6):
            note = 'F';
            is_sharp = true;
            break;
            
        case (7):
            note = 'G';
            break;
            
        case (8):
            note = 'G';
            is_sharp = true;
            break;
            
        case (9):
            note = 'A';
            break;
            
        case (10):
            note = 'A';
            is_sharp = true;
            break;
            
        case (11):
            note = 'B';
            break;
        }
        
        ascii = "";
        ascii += note;
        if (octave >= 10) {
            ascii += "10";
        }
        else {
            ascii += '0' + octave;
        }
        if (is_sharp) {
            ascii += '#';
        }
        r = 0;
    }
    
    return r;
}
