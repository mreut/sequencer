#ifndef __APPLICATION_MANAGER_HPP
#define __APPLICATION_MANAGER_HPP

/***** Includes *****/

#include <cstdint>
#include <mutex>
#include <string>

#include "MidiComposition.hpp"
#include "MidiOut.hpp"
#include "UserInterface.hpp"


/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum application_command {
    CMD_INVALID = 0,
    CMD_BPM = 'B',
    CMD_BPM_DECREMENT = '#',
    CMD_BPM_INCREMENT = '$',
    CMD_DELETE = 'D',
    CMD_INDEX = 'I',
    CMD_INDEX_DECREMENT = '!',
    CMD_INDEX_INCREMENT = '@',
    CMD_LOAD = 'L',
    CMD_MOVE = 'M',
    CMD_NOTE = 'N',
    CMD_PLAY = 'P',
    CMD_QUIT = 'Q',
    CMD_REPEAT = 'R',
    CMD_SAVE = 'S',
};


/***** Classes *****/

class ApplicationManager{
    public:
        ApplicationManager(
            void);
            
        ~ApplicationManager(
            void);
        
        void display_start(
            void);
            
        void display_stop(
            void);
        
        void display_refresh(
            void);
        
        void display_refresh_frame(
            void);
        
        void display_refresh_current_score(
            void);
            
        void start_play_current_score(
            void);
            
        void stop_play_current_score(
            void);
            
        void select_next_score(
            void);
            
        void select_past_score(
            void);
        
        void enter_command(
            application_command command,
            string entry);
        
    private:
        void play_main(
            void);
        
        uint32_t get_index(
            void);
            
        uint32_t get_display(
            void);
        
        MidiComposition comp_;
        
        MidiOut out_;
        
        UserInterface ui_;
        
        enum application_command command_;
        
        bool is_play_;
        
        thread play_thread_;
        
        mutex mutex_;
        
        uint32_t index_;
};

#endif
