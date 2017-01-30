#ifndef __APPLICATION_MANAGER_HPP
#define __APPLICATION_MANAGER_HPP

/***** Includes *****/

#include <cstdint>
#include <mutex>
#include <string>

#include "MidiComposition.hpp"
#include "MidiOut.hpp"
#include "MulticastSocket.hpp"
#include "UserInterface.hpp"


/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum application_command {
    CMD_INVALID = 0,
    CMD_BPM = 'B',
    CMD_BPM_DECREMENT = '#',
    CMD_BPM_INCREMENT = '$',
    CMD_CREATE_SCORE = 'C',
    CMD_TITLE_SCORE = 'T',
    CMD_VIEW_NEXT_SCORE = 'V',
    CMD_DELETE = 'D',
    CMD_INDEX = 'I',
    CMD_INDEX_DECREMENT = '!',
    CMD_INDEX_INCREMENT = '@',
    CMD_LOAD = 'L',
    CMD_ORIGIN = 'O',
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
        
        int32_t midi_out_start(
            string port_name);
        
        int32_t display_start(
            void);
        
        int32_t get_input(
            void);
        
        void echo_command(
            application_command command,
            string entry="");
        
        void enter_command(
            application_command command,
            string entry="");
            
        void start_master(
            void);
            
        void start_slave(
            void);
        
    private:
        void display_refresh(
            void);
        
        void display_refresh_info(
            void);
        
        void display_refresh_frame(
            void);
        
        void display_refresh_current_score(
            void);
            
        void display_refresh_command_line(
            void);
            
        void select_next_score(
            void);
            
        void select_past_score(
            void);
        
        void play_main(
            void);
            
        void slave_main(
            void);
        
        uint32_t get_index(
            void);
            
        uint32_t get_origin(
            void);
        
        MidiComposition comp_;
        
        MidiOut out_;
        
        MulticastSocket socket_;
        
        UserInterface ui_;
        
        enum application_command command_;
        
        bool is_running_;

        bool is_play_;
        
        thread play_thread_;
        
        thread slave_thread_;
        
        recursive_mutex mutex_;
        
        string command_line_;
        
        uint32_t index_;
        
        uint32_t origin_;
        
        uint32_t play_count_;
};

#endif
