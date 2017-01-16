/***** Includes *****/

#include <ncurses.h>
#include <cstdint>
#include <cctype>
#include <ctime>
#include <cerrno>
#include <algorithm> 
#include <functional>
#include <string>
#include <thread>

#include "MidiOut.hpp"
#include "MidiScore.hpp"
#include "UserInterface.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 8
#define SPACES_PER_NOTE 5
#define SPACES_PER_DIALOG 32


/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum input_command {
    CMD_INVALID = 0,
    CMD_BPM = 'B',
    CMD_CLEAR = 'C',
    CMD_INDEX = 'I',
    CMD_LOAD = 'L',
    CMD_NOTE = 'N',
    CMD_PLAY = 'P',
    CMD_QUIT = 'Q',
    CMD_SAVE = 'S',
};


/***** Local Variables *****/

thread _play_thread;

bool _is_play = false;


/***** Local Functions *****/

static int32_t _delay_ns(
    uint32_t num_nanosecs)
{
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    
    while (num_nanosecs >= 1e9) {
        num_nanosecs -= 1e9;
        ts.tv_sec += 1;
    }
    ts.tv_nsec = num_nanosecs;
    
    while (nanosleep(&ts, &ts) == -1) {
        if ( (errno == ENOSYS) || (errno == EINVAL)) {
            fprintf(stderr, "Error: nanosleep failed\r\n");
            return -1;
        }
    }
    
    return 0;
}

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
    MidiScore& score,
    uint32_t index)
{
    string str = "\0";
    int32_t row = 0;
    int32_t col = 0;
    uint8_t note = 0;
    
    while ((col <= ui.get_cols()) && (row <= (ui.get_rows() - 2))) {
        
        if (true != score.is_end(index)) {
            if (0 != score.get_note(index++, note)) {
                return -1;
            }
            else if (0 != note_to_ascii(note, str)) {
                    str = "";
            }
            
            if (5 > str.length()) {
                str += string(SPACES_PER_NOTE - str.length(), ' ');
            }
        }
        else {
            str = string(SPACES_PER_NOTE, ' ');
        }
        
        ui.print(row, col, A_NORMAL, str);
    
        if ((ui.get_cols() - (SPACES_PER_NOTE * 2)) <= col) {
            col = 0;
            row += 1;
        }
        else {
            col += SPACES_PER_NOTE;
        }
    }
    
    return 0;
}

static int32_t _play_main(
    UserInterface& ui,
    MidiScore& score,
    MidiOut& out)
{
    string str = "";
    int32_t row = 0;
    int32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint16_t bpm = 0;
    uint32_t delay = 0;
    
    while (true == _is_play) {
        score.get_bpm(bpm);
        delay = (uint32_t) ((60.0 / (double) bpm) * 1e9);
        
        if (0 != score.get_note(index++, note)) {
            return -1;
        }
        else if (0 != note_to_ascii(note, str)) {
            return -1;
        }
        
        ui.print(row, col, A_REVERSE | A_BLINK, str);
        if (MIDI_NOTE_REST != note) out.note_on(note, 100);
        _delay_ns(delay);
        if (MIDI_NOTE_REST != note) out.note_off(note, 100);
        ui.print(row, col, A_NORMAL, str);
        
        if (true == score.is_end(index)) {
            _print_score(ui, score, 0);
            index = 0;
            col = 0;
            row = 0;
        }
        else {
            if (col >= (ui.get_cols() - (SPACES_PER_NOTE*2))) {
                if (row >= (ui.get_rows() - 2)) {
                    _print_score(ui, score, index);
                    col = 0;
                    row = 0;
                }
                else {
                    col = 0;
                    row += 1;
                }
            }
            else {
                col += SPACES_PER_NOTE;
            }
        }
    }
    
    return 0;
}


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

    if (!argv[1]) {
        goto main_exit;
    } 
    else if (0 != output.open(argv[1])) {
        goto main_exit;
    }

#if 0
    score.set_note(0, 33);
    score.set_note(1, 24);
    score.set_note(2, 54);
    score.set_note(3, 36);
    score.set_note(4, 38);
    score.set_note(5, 39);
#endif

    while (false == is_exit_requested) {
        
        if (true == is_refresh_needed) {
            is_refresh_needed = false;
            ui.clear();
            _print_score(ui, score, index);
        }
        
        if (CMD_SAVE == cmd) {
            str = "[SAVE]: " + entry;
        }
        else if (CMD_LOAD == cmd) {
            str = "[LOAD]: " + entry;
        }
        else {
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
                score.get_note(index, note);
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
        }
        ui.print(ui.get_rows() - 1, 0, A_NORMAL, str);
        
        if (0 != ui.get_input(in)) {
            break;
        }
        else if (CMD_INVALID == cmd) {
            switch (in) {
            case (CMD_SAVE):
            case (CMD_LOAD):
                is_refresh_needed = true;
                // intentional fall through
                
            case (CMD_BPM):
            case (CMD_INDEX):
            case (CMD_NOTE):
                cmd = (enum input_command) in;
                break;
            
            case (CMD_CLEAR):
                score.clear_note(index);
                is_refresh_needed = true;
                break;
                
            case (CMD_PLAY):
                if (true == _is_play) {
                    _is_play = false;
                    _play_thread.join();
                }
                else {
                    _is_play = true;
                    _play_thread = thread(_play_main,
                                          ref(ui),
                                          ref(score),
                                          ref(output));
                }
                break;
                
            case (CMD_QUIT):
                if (true == _is_play) {
                    _is_play = false;
                    _play_thread.join();
                }
                is_exit_requested = true;
                break;
            }
        }
        else if (isprint(in)) {
            if ((CMD_SAVE == cmd) || (CMD_LOAD == cmd)) {
                if (SPACES_PER_DIALOG > entry.length()) entry += in;
            }
            else {
                if (7 > entry.length()) entry += in;
            }
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
                        is_refresh_needed = true;
                    }
                    break;
                    
                case (CMD_NOTE):
                    if (0 == ascii_to_note(entry, note)) {
                        score.set_note(index, note);
                        is_refresh_needed = true;
                    }
                    break;
                
                case (CMD_SAVE):
                    score.save(entry);
                    is_refresh_needed = true;
                    break;
                
                case (CMD_LOAD):
                    score.load(entry);
                    is_refresh_needed = true;
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
        else if (KEY_LEFT == in) {
            if (CMD_BPM == cmd) {
                if (1 < bpm) bpm--;
                entry = to_string(bpm);
            }
            else if (CMD_INDEX == cmd) {
                if (0 != index) index--;
                entry = to_string(index);
                is_refresh_needed = true;
            }
            else if (CMD_NOTE == cmd) {
                if (MIDI_NOTE_REST == note) note = MIDI_NOTE_MAX;
                else if (0 != note) note--;
                note_to_ascii(note, entry);
            }
        }
        else if (KEY_RIGHT == in) {
            if (CMD_BPM == cmd) {
                if (UINT16_MAX > bpm) bpm++;
                entry = to_string(bpm);
            }
            else if (CMD_INDEX == cmd) {
                if (MIDI_SCORE_LENGTH > index) index++;
                entry = to_string(index);
                is_refresh_needed = true;
            }
            else if (CMD_NOTE == cmd) {
                if (MIDI_NOTE_MAX > note) note++; else note = MIDI_NOTE_REST;
                note_to_ascii(note, entry);
            }
        }
    }
    
    r = 0;

main_exit:
    return r;
}
