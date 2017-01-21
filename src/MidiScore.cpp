/***** Includes *****/

#include <iostream>
#include <fstream>

#include "MidiScore.hpp"


/***** Class Methods *****/

MidiScore::MidiScore(
    void)
{
    for (uint32_t n = 0; n < MIDI_SCORE_LENGTH; n++) {
        this->score_[n].note = MIDI_NOTE_REST;
        this->score_[n].count = 0;
        this->score_[n].type = COUNT_MULTIPLY;
    }
    this->bpm_ = 60;
    this->last_note_ = -1;
}

int32_t MidiScore::save(
    string name)
{
    int32_t r = -1;
    ofstream out;
    
    out.open(name);
    if (true == out.is_open()) {
        out << to_string(this->bpm_) + "\n";
        for (int32_t n = 0; n <= this->last_note_; n++) {
            out << to_string(this->score_[n].note) + "\n";
            out << to_string(this->score_[n].count) + "\n";
            out << to_string(this->score_[n].type) + "\n";
            out << to_string(this->score_[n].count) + "\n";
        }
        out.close();
        r = 0;
    }
    
    return r;
}

int32_t MidiScore::load(
    string name)
{
    uint32_t n = 0;
    int32_t r = -1;
    string str = "";
    ifstream in;
    
    in.open(name);
    if (true == in.is_open()) {
        
        this->last_note_ = 0;
        getline(in, str);
        
        if (true != in.eof()) {
            this->bpm_ = stoi(str);
            
            while (n < MIDI_SCORE_LENGTH) {
                // first is midi note number
                getline(in, str);
                if ('\0' == str[0]) break;
                this->score_[n].note = stoi(str);
                // second is note count
                getline(in, str);
                if ('\0' == str[0]) break;
                this->score_[n].count = stoi(str);
                this->last_note_ = n++;
                // third is type
                getline(in, str);
                if ('\0' == str[0]) break;
                this->score_[n].type = (enum count_type) stoi(str);
                // fourth is count
                getline(in, str);
                if ('\0' == str[0]) break;
                this->score_[n].count = stoi(str);
            }
        }
        r = 0;
    }

    return r;
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
     else if (note > MIDI_NOTE_MAX) {
         note = MIDI_NOTE_REST;
     }
     
     this->mutex_.lock();
     if (((int32_t) index) > this->last_note_) {
         this->last_note_ = index;
     }
     if (0 == this->score_[index].count) {
         this->score_[index].count = 1;
     }
     this->score_[index].note = note;
     this->mutex_.unlock();
     
     return 0;
}
            
int32_t MidiScore::get_note(
    uint32_t index,
    uint8_t& note)
{
    if (index > MIDI_SCORE_LENGTH) {
        return -1;
    }
    
    this->mutex_.lock();
    note = this->score_[index].note;
    this->mutex_.unlock();
    
    return 0;
}

int32_t MidiScore::set_count(
    uint32_t index,
    enum count_type type,
    uint8_t count)
{
     if (index > MIDI_SCORE_LENGTH) {
         return -1;
     }
     else if ((COUNT_DIVIDE != type) && (COUNT_MULTIPLY != type)) {
         return -1;
     }
     else if (0 == count) { 
         count = 1;
     }
     
     this->mutex_.lock();
     if (((int32_t) index) > this->last_note_) {
        this->last_note_ = index;
     }
     this->score_[index].count = count;
     this->score_[index].type = type;
     this->mutex_.unlock();
     
     return 0;
}
            
int32_t MidiScore::get_count(
    uint32_t index,
    enum count_type& type,
    uint8_t& count)
{
    if (index > MIDI_SCORE_LENGTH) {
        return -1;
    }
    
    this->mutex_.lock();
    count = this->score_[index].count;
    type = this->score_[index].type;
    if ((COUNT_DIVIDE != type) && (COUNT_MULTIPLY != type)) {
        type = COUNT_MULTIPLY;
    }
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
    this->score_[index].count = 0;
    
    if (((int32_t) index) == this->last_note_) {
        for (int32_t n = (this->last_note_ - 1); n >= 0; n--) {
            if (0 != this->score_[n].count) {
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
