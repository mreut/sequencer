#include <ncurses.h>
#include <cstdint>
#include <cctype>
#include <algorithm> 
#include <string>
#include <thread>

#include "abstractions.hpp"
#include "MidiOut.hpp"
#include "MidiScore.hpp"
#include "UserInterface.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 8

/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum input_command {
    CMD_INVALID = 0,
    CMD_BPM = 'B',
    CMD_CLEAR = 'C',
    CMD_INDEX = 'I',
    CMD_NOTE = 'N',
    CMD_PLAY = 'P',
    CMD_QUIT = 'Q',
};


/***** Local Variables *****/

//thread _play_thread;

bool _is_play = false;


/***** Local Functions *****/

static bool _is_number(
    string& s)
{
    return !s.empty() && 
           find_if(s.begin(),
                   s.end(),
                   [](char c) { return !isdigit(c); }) == s.end();
}

static int32_t _print_score(
    UserInterface& ui,
    MidiScore& score)
{
    string note_string = "\0";
    uint32_t index = 0;
    int32_t row = 0;
    int32_t col = 0;
    uint8_t note = 0;
    
    while (true != score.is_end(index)) {
        
        if (0 != score.get_note(index++, &note)) {
            return -1;
        }
        else if (0 != note_to_ascii(note, note_string)) {
                note_string = "\0";
        }
    
        ui.print(row, col, A_NORMAL, note_string);
    
        if (col >= ui.get_cols()) {
            row += 1;
            col = 0;
        }
        else {
            col += 5;
        }
    }
    
    return 0;
}

#if 0
static int32_t _play_score(
    MidiScore& score,
    MidiOut& output)
{
    string note_string = "\0";
    int32_t row = 0;
    int32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint32_t delay = 0;
    
    while (true == _is_play) {
    
        score.get_bpm(&delay);
        delay = (uint32_t) ((60.0 / (double) delay) * 1e9);
        
        if (0 != score.get_note(index++, &note)) {
            return -1;
        }
        else if (0 != note_to_ascii(note, note_string)) {
            return -1;
        }

        ui.print(row, col, A_REVERSE | A_BLINK, note_string);
        output.note_on(note, 100);
        delay_ns(delay);
        output.note_off(note, 100);
        ui.print(row, col, A_NORMAL, note_string);
        
        if (col >= ui.get_cols()) {
            row += 1;
            col = 0;
        }
        else {
            col += 5;
        }
        
    }
    
    return 0;
}
#endif


/***** Global Functions *****/

int main(
    int argc,
    char* argv[])
{
    UserInterface ui;
    MidiScore score;
    MidiOut output;
    enum input_command cmd = CMD_INVALID;
    uint16_t index = 0;
    uint8_t note = 0;
    uint16_t bpm = 0;
    bool is_exit_requested = false;
    bool is_refresh_needed = true;
    string str = "";
    string entry = "";
    string tmp = "";
    int32_t in = 0;
    int32_t r = -1;

#if 0
    if (!argv[1]) {
        goto main_exit;
    } 
    else if (0 != output.open(argv[1])) {
        goto main_exit;
    }
#endif

    score.set_note(0, 33);
    score.set_note(1, 24);
    score.set_note(2, 54);
    score.set_note(3, 36);
    score.set_note(4, 38);
    score.set_note(5, 39);
    
    while (false == is_exit_requested) {
        
        if (true == is_refresh_needed) {
            is_refresh_needed = false;
            ui.clear();
            _print_score(ui, score);
        }
        
        if (true == _is_play) str = "[*]  "; else str = "[ ]  ";
        
        if (CMD_INDEX == cmd) {
            str += "[INDEX]: ";
            str += entry + string(SPACES_PER_PARAM - entry.length(), ' ');
        }
        else {
            str += " INDEX : ";
            tmp = to_string(index);
            str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
        }
        
        if (CMD_NOTE == cmd) {
            str += "[NOTE]: ";
            str += entry + string(SPACES_PER_PARAM - entry.length(), ' ');
        }
        else {
            str += " NOTE : ";
            score.get_note(index, &note);
            note_to_ascii(note, tmp);
            str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
        }

        if (CMD_BPM == cmd) {
            str += "[BPM]: ";
            str += entry + string(SPACES_PER_PARAM - entry.length(), ' ');
        }
        else {
            str += " BPM : ";
            score.get_bpm(bpm);
            tmp = to_string(bpm);
            str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
        }
        
        ui.print(ui.get_rows() - 1, 0, A_NORMAL, str);
        
        if (0 != ui.get_input(in)) {
            break;
        }
        else if (CMD_INVALID == cmd) {
            switch (in) {
            case (CMD_BPM):
            case (CMD_INDEX):
            case (CMD_NOTE):
                cmd = (enum input_command) in;
                break;
            
            case (CMD_CLEAR):
                score.clear_note(index);
                break;
                
            case (CMD_PLAY):
                _is_play = (false == _is_play) ? true : false;
                break;
                
            case (CMD_QUIT):
                is_exit_requested = true;
                break;
            }
        }
        else if (isprint(in)) {
            if (7 > entry.length()) entry += in;
        }
        else if (KEY_BACKSPACE == in) {
            if (0 < entry.length()) {
                entry.pop_back();
            }
            else {
                cmd = CMD_INVALID;
            }
        }
        else if ((KEY_ENTER == in) || (10 == in))  {
            switch (cmd) {
            if (0 != entry.length()) {
                case (CMD_BPM):
                    if (true == _is_number(entry)) {
                        bpm = stoi(entry);
                        score.set_bpm(bpm);
                    }
                    break;
                
                case (CMD_INDEX):
                    if (true == _is_number(entry)) {
                        index = stoi(entry);
                    }
                    break;
                    
                case (CMD_NOTE):
                    if (0 == ascii_to_note(entry, note)) {
                        score.set_note(index, note);
                        is_refresh_needed = true;
                    }
                    break;
                
                case (CMD_CLEAR):
                case (CMD_PLAY):
                case (CMD_QUIT):
                case (CMD_INVALID):
                    // do nothing
                    break;
                }
            }
            cmd = CMD_INVALID;
            entry = "";
        }
    }
    
    r = 0;

main_exit:
    return r;
}
