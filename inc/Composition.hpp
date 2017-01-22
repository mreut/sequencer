#ifndef __COMPOSITION_HPP
#define __COMPOSITION_HPP

/***** Includes *****/

#include <cstdint>
#include <list>
#include <mutex>

#include "MidiScore.hpp"


/***** Namespace *****/

using namespace std;


/***** Classes *****/

class Composition {
    public:
        Composition(
            void);
        
        ~Composition(
            void);
    
        int32_t create_next_score(
            void);
            
        int32_t create_past_score(
            void);
        
        void delete_score(
            void);
        
        MidiScore* get_score(
            void);
            
        MidiScore* get_next_score(
            void);
            
        MidiScore* get_past_score(
            void);
            
    private:
        list<MidiScore*> list_;
        list<MidiScore*>::iterator it_;
        mutex mutex_;
};

#endif
