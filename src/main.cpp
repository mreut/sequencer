/***** Includes *****/

#include <ncurses.h>
#include <cstdint>
#include <functional>
#include <list>
#include <string>
#include <thread>

#include "MidiOut.hpp"
#include "MidiScore.hpp"
#include "UserInterface.hpp"
#include "utility.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 10
#define SPACES_PER_NOTE 10
#define SPACES_PER_DIALOG 32
#define DISPLAY_START_ROW 2


/***** Namespace *****/

using namespace std;


/***** Enums *****/

enum input_command {
    CMD_INVALID = 0,
    CMD_BPM = 'B',
    CMD_DELETE = 'D',
    CMD_INDEX = 'I',
    CMD_LOAD = 'L',
    CMD_MOVE = 'M',
    CMD_NOTE = 'N',
    CMD_PLAY = 'P',
    CMD_QUIT = 'Q',
    CMD_REPEAT = 'R',
    CMD_SAVE = 'S',
};


/***** Structs *****/

struct application_parameters {
    MidiScore score;
    MidiOut out;
    UserInterface ui;
    enum input_command cmd;
    uint32_t display;
    uint32_t index;
    string entry;
    thread play_thread;
    bool is_ui_refresh;
};


/***** Local Variables *****/

static bool _is_play = false;


/***** Local Functions *****/

static void _print_frame(
    struct application_parameters& app)
{
    const uint32_t max_line_len = app.ui.get_cols() - SPACES_PER_NOTE;
    const uint32_t max_row = app.ui.get_rows() - 3;
    string str = "";
    uint8_t tmp = 0;
    
    app.score.get_repeat(tmp);
    str = "0:" + to_string(tmp) + " ";
    app.ui.print(0, 0, A_NORMAL, str);
    str = string(max_line_len, '=');
    app.ui.print(1, 0, A_NORMAL, str);
    app.ui.print(max_row + 1, 0, A_NORMAL, str);
}

static int32_t _print_score(
    struct application_parameters& app,
    uint32_t offset)
{
    const uint32_t max_line_len = app.ui.get_cols() - SPACES_PER_NOTE;
    const uint32_t max_row = app.ui.get_rows() - 3;
    uint32_t row = DISPLAY_START_ROW;
    enum count_type type;
    string str = "";
    string line = "";
    uint8_t note = 0;
    uint8_t count = 0;
    
    _print_frame(app);
    
    while (row <= max_row) {
        if (true != app.score.is_end(offset)) {
            // grab the next note
            if ((0 != app.score.get_note_count(offset++, note, type, count)) ||
                (!note_count_to_ascii(note, type, count, str))) {
                return -1;
            }
            else if (SPACES_PER_NOTE > str.length()) {
                // pad with spaces to fully erase previous
                str += string(SPACES_PER_NOTE - str.length(), ' ');
            }
        }
        else {
            // pad with spaces to fully erase previous
            str = string(SPACES_PER_NOTE, ' ');
        }
        
        line += str;
        
        if (max_line_len <= line.length()) {
            // print the current row
            app.ui.print(row, 0, A_NORMAL, line);
            line = "";
            row += 1;
        }
    }
    
    return 0;
}

static void _print_command_line(
    struct application_parameters& app)
{
    enum count_type type;
    string str = "";
    string tmp = "";
    uint16_t bpm = 0;
    uint16_t repeat = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    
    if (CMD_SAVE == app.cmd) {
        str = "[SAVE]: " + app.entry;
        str += string(app.ui.get_cols() - str.length(), ' ');
    }
    else if (CMD_LOAD == app.cmd) {
        str = "[LOAD]: " + app.entry;
        str += string(app.ui.get_cols() - str.length(), ' ');
    }
    else if (CMD_MOVE == app.cmd) {
        str = "[MOVE]: " + app.entry;
        str += string(app.ui.get_cols() - str.length(), ' ');
    }
    else if (CMD_REPEAT == app.cmd) {
        str = "[REPEAT]: " + app.entry;
        str += string(app.ui.get_cols() - str.length(), ' ');
    }
    else {
        if (true == _is_play) str = "[*]  "; else str = "[ ]  ";
        
        if (CMD_BPM == app.cmd) {
            str += "[BPM]: ";
            str += app.entry;
            str += string(SPACES_PER_PARAM - app.entry.length(), ' ');
        }
        else {
            str += " BPM : ";
            app.score.get_bpm(bpm);
            str += to_string(bpm);
            str += string(SPACES_PER_PARAM - tmp.length(), ' ');
        }
        
        if (CMD_INDEX == app.cmd) {
            str += "[INDEX]: ";
            str += app.entry;
            str += string(SPACES_PER_PARAM - app.entry.length(), ' ');
        }
        else {
            str += " INDEX : ";
            str += to_string(app.index);
            str += string(SPACES_PER_PARAM - tmp.length(), ' ');
        }
        
        if (CMD_NOTE == app.cmd) {
            str += "[NOTE]: ";
            str += app.entry;
            str += string(SPACES_PER_PARAM - app.entry.length(), ' ');
        }
        else {
            str += " NOTE : ";
            app.score.get_note(app.index, note);
            app.score.get_count(app.index, type, count);
            note_to_ascii(note, tmp);
            if (1 < count) {
                tmp += (COUNT_DIVIDE == type) ? "/" :
                       (COUNT_MULTIPLY == type) ? "*" :
                       "?";
                tmp += to_string(count);
            }
            str += tmp;
            str += string(SPACES_PER_PARAM - tmp.length(), ' ');
        }
    }
    app.ui.print(app.ui.get_rows() - 1, 0, A_NORMAL, str);
}

static void _handle_backspace(
    struct application_parameters& app)
{
    if (0 < app.entry.length()) {
        app.entry.pop_back();
    }
    else {
        app.cmd = CMD_INVALID;
    }
}

static void _handle_enter(
    struct application_parameters& app)
{
    enum count_type type;
    uint16_t bpm = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    
    if (0 != app.entry.length()) {
        switch (app.cmd) {
        case (CMD_BPM):
            if (is_number(app.entry)) {
                bpm = stoi(app.entry);
                app.score.set_bpm(bpm);
            }
            break;
        
        case (CMD_INDEX):
            if (is_number(app.entry)) {
                app.index = stoi(app.entry);
            }
            break;
        
        case (CMD_MOVE):
            if (is_number(app.entry)) {
                app.display = stoi(app.entry);
                app.index = app.display;
                app.is_ui_refresh = true;
            }
            break;
        
        case (CMD_REPEAT):
            if (is_number(app.entry)) {
                app.score.set_repeat(stoi(app.entry));
                app.is_ui_refresh = true;
            }
            break;
        
        case (CMD_NOTE):
            if (ascii_to_note_count(app.entry, note, type, count)) {
                app.score.set_note_count(app.index++, note, type, count);
                app.is_ui_refresh = true;
            }
            break;
        
        case (CMD_SAVE):
            app.score.save(app.entry);
            app.is_ui_refresh = true;
            break;
        
        case (CMD_LOAD):
            app.score.load(app.entry);
            app.is_ui_refresh = true;
            break;
        
        case (CMD_DELETE):
        case (CMD_PLAY):
        case (CMD_QUIT):
        case (CMD_INVALID):
            // do nothing
            break;
        }
    }
    app.cmd = CMD_INVALID;
    app.entry = "";
}

static void _handle_left(
    struct application_parameters& app)
{
    uint16_t bpm = 0;
    uint8_t note = 0;
    string str = "";
    
    if (CMD_BPM == app.cmd) {
        app.score.get_bpm(bpm);
        if (1 < bpm) app.score.set_bpm(--bpm);
        app.entry = to_string(bpm);
    }
    else if (CMD_INDEX == app.cmd) {
        if (0 != app.index) app.index--;
        app.entry = to_string(app.index);
    }
    else if (CMD_NOTE == app.cmd) {
        app.score.get_note(app.index, note);
        note = (MIDI_NOTE_REST == note) ? MIDI_NOTE_MAX : note - 1;
        note_to_ascii(note, str);
        app.entry = str;
    }
}

static void _handle_right(
    struct application_parameters& app)
{
    uint16_t bpm = 0;
    uint8_t note = 0;
    string str = "";
    
    if (CMD_BPM == app.cmd) {
        app.score.get_bpm(bpm);
        if (UINT16_MAX > bpm) app.score.set_bpm(++bpm);
        app.entry = to_string(bpm);
    }
    else if (CMD_INDEX == app.cmd) {
        if (MIDI_SCORE_LENGTH > app.index) app.index++;
        app.entry = to_string(app.index);
    }
    else if (CMD_NOTE == app.cmd) {
        app.score.get_note(app.index, note);
        note = ((MIDI_NOTE_REST == note) || (MIDI_NOTE_MAX == note)) ?
                MIDI_NOTE_REST : note + 1;
        note_to_ascii(note, str);
        app.entry = str;
    }
}

static void _handle_character(
    struct application_parameters& app,
    uint8_t ch)
{
    if ((CMD_SAVE == app.cmd) || (CMD_LOAD == app.cmd)) {
        if (SPACES_PER_DIALOG > app.entry.length()) {
            app.entry += ch;
        }
    }
    else if ((CMD_BPM == app.cmd) || (CMD_INDEX == app.cmd)) {
        // filter out non-numeric characters
        if ((isdigit(ch)) && ((SPACES_PER_PARAM - 2) > app.entry.length())) {
            app.entry += ch;
        }
    }
    else {
        if ((SPACES_PER_PARAM - 2) > app.entry.length()) {
            app.entry += ch;
        }
    }
}

static int32_t _play_main(
    struct application_parameters& app)
{
    enum count_type type;
    string str = "";
    uint32_t max_col = 0;
    uint32_t max_row = 0;
    uint32_t row = DISPLAY_START_ROW;
    uint32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    uint16_t bpm = 0;
    uint8_t repeat = 0;
    uint8_t iter = 0;
    uint64_t delay = 0;

    app.score.get_repeat(repeat);
    str = to_string(iter) + ":" + to_string(repeat) + " ";
    app.ui.print(0, 0, A_NORMAL, str);
    
    while (true == _is_play) {
        app.score.get_bpm(bpm);
        app.score.get_repeat(repeat);
        max_col = app.ui.get_cols() - SPACES_PER_NOTE;
        max_row = app.ui.get_rows() - 3;
        
        if ((0 != app.score.get_note_count(index++, note, type, count)) || 
            (!note_count_to_ascii(note, type, count, str))) {
            return -1;
        }
        
        delay = (uint32_t) ((60.0 / (double) bpm) * 1e9);
        delay = (COUNT_DIVIDE == type) ? delay / count : delay * count;
        
        app.ui.print(row, col, A_REVERSE | A_BLINK, str);
        if (MIDI_NOTE_REST != note) app.out.note_on(note, 100);
        delay_ns(delay);
        if (MIDI_NOTE_REST != note) app.out.note_off(note, 100);
        app.ui.print(row, col, A_NORMAL, str);
        
        if (true == app.score.is_end(index)) {
            _print_score(app, 0);
            iter = (repeat <= iter) ? 0 : iter + 1;
            str = to_string(iter) + ":" + to_string(repeat) + " ";
            app.ui.print(0, 0, A_NORMAL, str);
            index = 0;
            col = 0;
            row = DISPLAY_START_ROW;
        }
        else if (col >= max_col) {
            if (row >= max_row) {
                _print_score(app, index);
                col = 0;
                row = DISPLAY_START_ROW;
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
    
    return 0;
}


/***** Global Functions *****/

int main(
    int argc,
    char* argv[])
{
    struct application_parameters app;
    bool is_exit_requested = false;
    int32_t in = 0;
    int32_t r = -1;
    
    app.index = 0;
    app.display = 0;
    app.is_ui_refresh = true;
    app.cmd = CMD_INVALID;
    
    if (!argv[1]) {
        goto main_exit;
    } 
    else if (0 != app.out.open(argv[1])) {
        goto main_exit;
    }

    app.ui.start();

    while (false == is_exit_requested) {
        
        if (true == app.is_ui_refresh) {
            app.is_ui_refresh = false;
            app.ui.clear();
            _print_score(app, app.display);
        }
        
        _print_command_line(app);
        
        if (0 != app.ui.get_input(in)) {
            break;
        }
        else if (CMD_INVALID == app.cmd) {
            switch (in) {
            case (CMD_SAVE):
            case (CMD_LOAD):
             case (CMD_MOVE):
                app.is_ui_refresh = true;
                // intentional fall through
                
            case (CMD_BPM):
            case (CMD_REPEAT):
            case (CMD_INDEX):
            case (CMD_NOTE):
                app.cmd = (enum input_command) in;
                break;
            
            case (CMD_DELETE):
                app.score.clear_note(app.index);
                app.is_ui_refresh = true;
                break;
                
            case (CMD_PLAY):
                if (true == _is_play) {
                    _is_play = false;
                    app.play_thread.join();
                    app.is_ui_refresh = true;
                }
                else {
                    _is_play = true;
                    app.play_thread = thread(_play_main, ref(app));
                }
                break;
                
            case (CMD_QUIT):
                if (true == _is_play) {
                    _is_play = false;
                    app.play_thread.join();
                }
                is_exit_requested = true;
                break;
            }
        }
        else if (isprint(in)) {
            _handle_character(app, in);
        }
        else if (KEY_BACKSPACE == in) {
            _handle_backspace(app);
        }
        else if ((KEY_ENTER == in) || (10 == in)) {
            _handle_enter(app);
        }
        else if (KEY_LEFT == in) {
            _handle_left(app);
        }
        else if (KEY_RIGHT == in) {
            _handle_right(app);
        }
    }
    
    r = 0;

main_exit:
    return r;
}
