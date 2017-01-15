/***** Inlcudes *****/

#include "MidiScore.hpp"


/***** Defines *****/

#define MIDI_NOTE_MAX 0x7F


/***** Class Methods *****/

MidiScore::MidiScore(
    void)
{
    for (uint32_t n = 0; n < MIDI_SCORE_LENGTH; n++) {
        this->score_[n].note = MIDI_NOTE_REST;
        this->score_[n].is_set = false;
    }
    this->bpm_ = 60;
    this->last_note_ = -1;
}

int32_t MidiScore::set_bpm(
    uint16_t bpm)
{
    if (bpm == 0) {
        return -1;
    }
    
    this->mutex_.lock();
    this->bpm_ = bpm;
    this->mutex_.unlock();
    
    return 0;
}

int32_t MidiScore::get_bpm(
    uint16_t& bpm)
{
    this->mutex_.lock();
    bpm = this->bpm_;
    this->mutex_.unlock();
    
    return 0;
}

        
int32_t MidiScore::set_note(
    uint32_t index,
    uint8_t note)
{
     if (index > MIDI_SCORE_LENGTH) {
         return -1;
     }
     
     if (note > MIDI_NOTE_MAX) {
         note = MIDI_NOTE_REST;
     }
     
     if (((int32_t) index) > this->last_note_) {
         this->last_note_ = index;
     }
     
     this->mutex_.lock();
     this->score_[index].note = note;
     this->score_[index].is_set = true;
     this->mutex_.unlock();
     
     return 0;
}
            
int32_t MidiScore::get_note(
    uint32_t index,
    uint8_t* p_note)
{
    if (index > MIDI_SCORE_LENGTH) {
        return -1;
    }
    
    this->mutex_.lock();
    *p_note = (true == this->score_[index].is_set) ?
              this->score_[index].note : 
              MIDI_NOTE_REST;
    this->mutex_.unlock();
    
    return 0;
}

int32_t MidiScore::clear_note(
    uint32_t index)
{
    if (index > MIDI_SCORE_LENGTH) {
        return -1;
    }
    
    this->mutex_.lock();
    this->score_[index].note = MIDI_NOTE_REST;
    this->score_[index].is_set = false;
    
    if (((int32_t) index) == this->last_note_) {
        for (int32_t n = (this->last_note_ - 1); n >= 0; n--) {
            if (true == this->score_[n].is_set) {
                this->last_note_ = n;
                break;
            }
            else if (0 == n) {
                this->last_note_ = -1;
            }
        }
    }
    this->mutex_.unlock();
    
    return 0;
}

bool MidiScore::is_end(
    uint32_t index)
{
    bool is_end = false;
    
    this->mutex_.lock();
    is_end = (((int32_t) index) > this->last_note_) ? true : false;
    this->mutex_.unlock();
    
    return is_end;
}


/***** Global Functions *****/

int32_t ascii_to_note(
    string& ascii,
    uint8_t& note)
{
    uint8_t tmp = 0;
    uint8_t octave = 0;
    int32_t r = 0;
    
    if ('-' == ascii[0]) {
        note = MIDI_NOTE_REST;
    }
    else if (isalpha(ascii[0])) {
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
            r = -1;
        }
        
        if (0 == r) {
            if (isdigit(ascii[1])) {
                if (isdigit(ascii[2])) {
                    octave = 10;
                }
                else {
                    octave = ascii[1] - '0';
                }
            
                tmp += 12 * octave;
                if (('#' == ascii[2]) || ('#' == ascii[3])) {
                    tmp += 1;
                }
                note = tmp;
            }
            else {
                r = -1;
            }
        }
    }
    
    return r;
}

int32_t note_to_ascii(
    uint8_t note,
    string& ascii)
{
    int32_t octave = 0;
    int32_t r = 0;
    bool is_sharp = false;

    if (note == MIDI_NOTE_REST) {
        ascii = '-';
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
    }
    else {
        r = -1;
    }
    
    return r;
}
