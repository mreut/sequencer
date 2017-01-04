/***** Inlcudes *****/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include "abstractions.hpp"


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
    uint8_t* p_string,
    uint8_t* p_note)
{
    uint8_t note = 0;
    uint8_t octave = 0;
    
    switch(p_string[0]) {
    case ('A'):
        note = 9;
        break;
    case ('B'):
        note = 11;
        break;
    case ('C'):
        note = 0;
        break;
    case ('D'):
        note = 2;
        break;
    case ('E'):
        note = 4;
        break;
    case ('F'):
        note = 5;
        break;
    case ('G'):
        note = 7;
        break;
    default:
        return -1;
    }
    
    if (isdigit(p_string[1])) {
        if (isdigit(p_string[2])) {
            octave = 10;
        }
        else {
            octave = p_string[1] - '0';
        }
    }
    else {
        return -1;
    }
    
    note += 12 * octave;
    
    if (('#' == p_string[2]) || ('#' == p_string[3])) {
        note += 1;
    }
    
    if (note > MIDI_NOTE_MAX) {
        return -1;
    }
    
    *p_note = note;
    
    return 0;
}

int32_t note_to_ascii(
    uint8_t note,
    uint8_t* p_string)
{
    int32_t octave = 0;
    uint8_t mod = '\0';

    if (note > MIDI_NOTE_MAX) {
        return -1;
    }
    
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
        mod = '#';
        break;
    
    case (2):
        note = 'D';
        break;
        
    case (3):
        note = 'D';
        mod = '#';
        break;
        
    case (4):
        note = 'E';
        break;
        
    case (5):
        note = 'F';
        break;
        
    case (6):
        note = 'F';
        mod = '#';
        break;
        
    case (7):
        note = 'G';
        break;
        
    case (8):
        note = 'G';
        mod = '#';
        break;
        
    case (9):
        note = 'A';
        break;
        
    case (10):
        note = 'A';
        mod = '#';
        break;
        
    case (11):
        note = 'B';
        break;
    }
    
    if (octave < 10) {
        p_string[0] = note;
        p_string[1] = '0' + octave;
        p_string[2] = mod;
        p_string[3] = '\0';
        p_string[4] = '\0';
    }
    else {
        p_string[0] = note;
        p_string[1] = '1';
        p_string[2] = '0';
        p_string[0] = note;
        p_string[5] = '\0';
    }
    
    return 0;
}
