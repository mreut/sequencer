/***** Includes *****/

#include "Composition.hpp"


/***** Class Methods *****/

Composition::Composition(
    void)
{
    MidiScore* p_score = new MidiScore();
    
    this->list_.push_back(p_score);
    this->it_ = this->list_.begin();
}

Composition::~Composition(
    void)
{
    list<MidiScore*>::iterator it;
    for (it = this->list_.begin(); it != this->list_.end(); ++it) {
        delete *it;
    }
}
#if 0
int32_t Composition::create_next_score(
    void)
{
    this->mutex_.lock();
    list<MidiScore>::iterator it = this->it_;
    it++;
    this->list_.insert(it, MidiScore());
    this->mutex_.unlock();
    
    return 0;
}
    
int32_t Composition::create_past_score(
    void)
{
    this->mutex_.lock();
    this->list_.insert(this->it_, MidiScore());
    this->mutex_.unlock();

    return 0;
}
#endif
void Composition::delete_score(
    void)
{
    // TODO: Want to avoid problems of disappearing shared memory...
    return;
}

MidiScore* Composition::get_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    p_score = *(this->it_);
    this->mutex_.unlock();
    
    return p_score;
}

MidiScore* Composition::get_next_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    this->it_++;
    p_score = *(this->it_);
    this->mutex_.unlock();
    
    return p_score;
}
    
MidiScore* Composition::get_past_score(
    void)
{
    MidiScore* p_score = NULL;
    
    this->mutex_.lock();
    this->it_--;
    p_score = *(this->it_);
    this->mutex_.unlock();
    
    return p_score;
}
