/***** Inlcudes *****/

#include "MidiScore.hpp"
#include "abstractions.hpp"


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
