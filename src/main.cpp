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

#define SPACES_PER_PARAM 10
#define SPACES_PER_NOTE 10
#define SPACES_PER_DIALOG 32


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

static int32_t _delay_ns(
    uint64_t num_nanosecs)
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
    struct application_parameters& app,
    uint32_t offset)
{
    enum count_type type;
    string str = "";
    int32_t row = 0;
    int32_t col = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    uint32_t index = offset;
    
    while ((col <= app.ui.get_cols()) && (row <= (app.ui.get_rows() - 2))) {
        
        if (true != app.score.is_end(index)) {
            if ((0 != app.score.get_note(index, note)) ||
                (0 != app.score.get_count(index++, type, count))) {
                return -1;
            }
            else if (0 != note_to_ascii(note, str)) {
                str = "";
            }
            else if (1 < count) {
                if (COUNT_DIVIDE == type) {
                    str += "/" + to_string(count);
                }
                else if (COUNT_MULTIPLY == type) {
                    str += "*" + to_string(count);
                }
            }
            
            if (SPACES_PER_NOTE > str.length()) {
                str += string(SPACES_PER_NOTE - str.length(), ' ');
            }
        }
        else {
            str = string(SPACES_PER_NOTE, ' ');
        }
        
        app.ui.print(row, col, A_NORMAL, str);
    
        if ((app.ui.get_cols() - (SPACES_PER_NOTE * 2)) <= col) {
            col = 0;
            row += 1;
        }
        else {
            col += SPACES_PER_NOTE;
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
    uint8_t note = 0;
    uint8_t count = 0;
    
    if (CMD_SAVE == app.cmd) {
        str = "[SAVE]: " + app.entry;
        str += string(SPACES_PER_DIALOG - str.length(), ' ');
    }
    else if (CMD_LOAD == app.cmd) {
        str = "[LOAD]: " + app.entry;
        str += string(SPACES_PER_DIALOG - str.length(), ' ');
    }
    else if (CMD_MOVE == app.cmd) {
        str = "[MOVE]: " + app.entry;
        str += string(SPACES_PER_DIALOG - str.length(), ' ');
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

static bool _score_add_entry(
    MidiScore& score,
    uint32_t index,
    string entry)
{
    int32_t r = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    string tmp = "";
    bool success = false;
    
    if (((2 <= (r = entry.find("*")))) && 
        ((r + 1) < (int32_t) entry.length())) {
        tmp = entry.substr(r + 1);
        if (true == _is_number(tmp)) {
            count = stoi(tmp);
            score.set_count(index, COUNT_MULTIPLY, count);
        }
    }
    else if (((2 <= (r = entry.find("/")))) && 
             ((r + 1) < (int32_t) entry.length())) {
        tmp = entry.substr(r + 1);
        if (true == _is_number(tmp)) {
            count = stoi(tmp);
            score.set_count(index, COUNT_DIVIDE, count);
        }
    }
    if (0 == ascii_to_note(entry, note)) {
        score.set_note(index, note);
        success = true;
    }
    
    return success;
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
    uint16_t bpm = 0;
    
    if (0 != app.entry.length()) {
        switch (app.cmd) {
        case (CMD_BPM):
            if (_is_number(app.entry)) {
                bpm = stoi(app.entry);
                app.score.set_bpm(bpm);
            }
            break;
        
        case (CMD_INDEX):
            if (_is_number(app.entry)) {
                app.index = stoi(app.entry);
            }
            break;
        
        case (CMD_MOVE):
            if (_is_number(app.entry)) {
                app.display = stoi(app.entry);
                app.index = app.display;
                app.is_ui_refresh = true;
            }
            break;
        
        case (CMD_NOTE):
            if (_score_add_entry(app.score, app.index, app.entry)) {
                app.index++;
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
    int32_t row = 0;
    int32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    uint16_t bpm = 0;
    uint64_t delay = 0;
    
    while (true == _is_play) {
        app.score.get_bpm(bpm);
        delay = (uint32_t) ((60.0 / (double) bpm) * 1e9);
        
        if ((0 != app.score.get_note(index, note)) || 
            (0 != app.score.get_count(index++, type, count))) {
            return -1;
        }
        else if (0 != note_to_ascii(note, str)) {
            return -1;
        }
        else if (1 < count) {
            if (COUNT_DIVIDE == type) {
                str += "/" + to_string(count);
                delay /= count;
            }
            else if (COUNT_MULTIPLY == type) {
                str += "*" + to_string(count);
                delay *= count;
            }
        }
        
        app.ui.print(row, col, A_REVERSE | A_BLINK, str);
        if (MIDI_NOTE_REST != note) app.out.note_on(note, 100);
        _delay_ns(delay);
        if (MIDI_NOTE_REST != note) app.out.note_off(note, 100);
        app.ui.print(row, col, A_NORMAL, str);
        
        if (true == app.score.is_end(index)) {
            _print_score(app, 0);
            index = 0;
            col = 0;
            row = 0;
        }
        else if (col >= (app.ui.get_cols() - (SPACES_PER_NOTE*2))) {
            if (row >= (app.ui.get_rows() - 2)) {
                _print_score(app, index);
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
