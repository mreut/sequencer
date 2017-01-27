/***** Includes *****/

#include "MidiComposition.hpp"


/***** Class Methods *****/

MidiComposition::MidiComposition(
    void)
{
    MidiScore* p_score = new MidiScore();
    
    this->list_.push_back(p_score);
    this->it_ = this->list_.begin();
}

MidiComposition::~MidiComposition(
    void)
{
    list<MidiScore*>::iterator it;
    for (it = this->list_.begin(); it != this->list_.end(); ++it) {
        delete *it;
    }
}

int32_t MidiComposition::create_next_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    list<MidiScore*>::iterator it = this->it_;
    it++;
    p_score = new MidiScore();
    this->list_.insert(it, p_score);
    this->mutex_.unlock();
    
    return 0;
}
    
int32_t MidiComposition::create_past_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    p_score = new MidiScore();
    this->list_.insert(this->it_, p_score);
    this->mutex_.unlock();

    return 0;
}

void MidiComposition::delete_score(
    void)
{
    // TODO: Want to avoid problems of disappearing shared memory...
    return;
}

MidiScore* MidiComposition::get_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    p_score = *(this->it_);
    this->mutex_.unlock();
    
    return p_score;
}

void MidiComposition::next_score(
    void)
{
    this->mutex_.lock();
    this->it_++;
    if (this->it_ == this->list_.end()) {
        this->it_ = this->list_.begin();
    }
    this->mutex_.unlock();
}
    
void MidiComposition::past_score(
    void)
{
    this->mutex_.lock();
    this->it_--;
    this->mutex_.unlock();
}
