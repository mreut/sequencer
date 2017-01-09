/***** Includes *****/

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <ncurses.h>

#include "input.hpp"
#include "abstractions.hpp"


/***** Defines *****/

#define INPUT_KEY_PLAY 'P'
#define INPUT_KEY_BPM 'B'
#define INPUT_KEY_INDEX 'I'
#define INPUT_KEY_NOTE 'N'
#define INPUT_KEY_QUIT 'Q'
#define INPUT_KEY_CLEAR 'C'


/***** Namespace *****/

using namespace std;


/***** Structs *****/

struct input_command {
    uint8_t command;
    uint32_t data;
};


/***** Local Variables *****/

mutex _mutex;

thread _thread;

queue<struct input_command> _fifo;

bool _is_initialized = false;

bool _is_active = false;


/***** Local Functions *****/

int32_t _user_input_add(
    uint8_t command,
    uint32_t data)
{
    struct input_command in;
    
    in.command = command;
    in.data = data;
    
    _mutex.lock();
    _fifo.push(in);
    _mutex.unlock();
    
    return 0;
}

void _user_input(
    void)
{
    string entry = "\0";
    bool is_play = false;
    int32_t ch = 0;
    uint8_t cmd = INPUT_COMMAND_INVALID;
    uint8_t note = 0;
    int32_t rows = 0;
    int32_t cols = 0;
    uint32_t data = 0;
    
    _mutex.lock();
    _is_active = true;
    _mutex.unlock();
    
    while (true == _is_active) {
        getmaxyx(stdscr, rows, cols);
        
        ch = getch();
        
        if ((INPUT_COMMAND_INVALID == cmd) && (isalpha(ch))) {
            
            ch &= ~0x20;
            
            switch (ch) {
            case (INPUT_KEY_BPM):
                cmd = INPUT_COMMAND_SET_BPM;
                break;
                
            case (INPUT_KEY_INDEX):
                cmd = INPUT_COMMAND_SET_INDEX;
                break;
                
            case (INPUT_KEY_NOTE):
                cmd = INPUT_COMMAND_SET_NOTE;
                break;
                
            case (INPUT_KEY_CLEAR):
                break;
                
            case (INPUT_KEY_PLAY):
                if (false == is_play) {
                    _user_input_add(INPUT_COMMAND_START_PLAY, 0);
                    is_play = true;
                }
                else {
                    _user_input_add(INPUT_COMMAND_STOP_PLAY, 0);
                    is_play = false;
                }
                break;
                
            case (INPUT_KEY_QUIT):
                _user_input_add(INPUT_COMMAND_EXIT, 0);
                break;
            }
        }
        else if (isprint(ch)) {
            if (isalpha(ch)) ch &= ~0x20;
            mvwaddch(stdscr, rows-1, entry.length(), ch);
            entry += ch;
        }
        else if (KEY_BACKSPACE == ch) {
            if (0 < entry.length()) {
                entry.pop_back();
                mvwaddch(stdscr, rows-1, entry.length(), ' ');
            }
            else {
                cmd = INPUT_COMMAND_INVALID;
            }
        }
        else if ((KEY_ENTER == ch) || (10 == ch)) {
            switch (cmd) {
            case (INPUT_KEY_BPM):
            case (INPUT_KEY_INDEX):
                data = stol(entry);
                _user_input_add(cmd, data);
                break;
            
            case (INPUT_KEY_NOTE):
                if (0 == ascii_to_note(entry, note)) {
                    data = note;
                    _user_input_add(cmd, data);
                }
                break;
            }
            entry = "\0";
        }
    }
}


/***** Global Functions *****/

int32_t input_start(
    void)
{
    if (true == _is_initialized) {
        return -1;
    }
    
    _is_initialized = true;
    _thread = thread(_user_input);
    
    return 0;
}

int32_t input_stop(
    void)
{
    if (false == _is_initialized) {
        return 0;
    }
    
    _mutex.lock();
    if (true == _is_active) {
        _is_active = false;
        
        _thread.join();
    }
    _mutex.unlock();
    
    return 0;
}

int32_t input_get(
    uint8_t& command,
    uint8_t& data)
{
    struct input_command in;
    int32_t r = -1;
    
    _mutex.lock();
    if (0 != _fifo.size()) {
        r = 0;
        in = _fifo.front();
        command = in.command;
        data = in.data;
        _fifo.pop();
    }
    _mutex.unlock();
    
    return r;
}
