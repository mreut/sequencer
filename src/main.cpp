#include <ncurses.h>
#include <cstdint>
#include <cctype>
#include <thread>

#include "abstractions.hpp"
#include "MidiOut.hpp"
#include "MidiScore.hpp"
#include "UserInterface.hpp"


/***** Namespace *****/

using namespace std;


/***** Local Variables *****/

thread _play_thread;

bool _is_play = false;


/***** Local Functions *****/

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
    uint32_t index = 0;
    uint8_t note = 0;
    uint32_t bpm = 0;
    bool is_exit_requested = false;
    bool is_refresh_needed = true;
    string str = "";
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

    score.get_bpm(&bpm);
    score.set_note(0, 34);
    score.set_note(1, 24);
    score.set_note(2, 54);
    score.set_note(3, 36);
    score.set_note(4, 38);
    score.set_note(5, 39);
    
    while (false == is_exit_requested) {
        
        if (true == is_refresh_needed) {
            ui.clear();
            _print_score(ui, score);
        }
        
        sleep(1);
        
        ui.clear();
        if (0 == ui.get_input(in)) {
            str += (char) in;
            ui.print(2, 0, A_NORMAL, str);
        }
        if (0 == ui.get_input(in)) {
            str += (char) in;
            ui.print(2, 0, A_NORMAL, str);
        }
        sleep(1);
        
        _print_score(ui, score);
        sleep(1);
        is_exit_requested = true;
    }
    
    r = 0;

main_exit:
    return r;
}
