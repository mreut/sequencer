#include <sys/ioctl.h>
#include <ncurses.h>
#include <cstdint>
#include <cctype>

#include "abstractions.hpp"
#include "MidiOut.hpp"
#include "MidiScore.hpp"


/***** Namespace *****/

using namespace std;


/***** Defines *****/

#define PRINT_BPM(y_dim, x_dim, bpm)            \
    do {                                        \
        mvprintw(y_dim-2, x_dim-5, "     ");    \
        mvprintw(y_dim-2, x_dim-5, "%d", bpm);  \
    } while (0)


/***** Local Functions *****/

int32_t _init_display(
    void)
{
    initscr();
    raw();                  /* Line buffering disabled */
    keypad(stdscr, TRUE);   /* We get F1, F2 etc.. */
    noecho();
    
    refresh();
    curs_set(0);
    
    return 0;
}

static int32_t _print_score(
    MidiScore& score)
{
    string note_string = "\0";
    uint32_t index = 0;
    int32_t rows = 0;
    int32_t cols = 0;
    uint8_t note = 0;
    
    getmaxyx(stdscr, rows, cols);
    
    for (int32_t m = 0; (m < (rows-1)) && (true != score.is_end(index)); m++) {
        
        for (int32_t n = 0; (n < 8) && (true != score.is_end(index)); n++) {

            if (0 != score.get_note(index++, &note)) {
                return -1;
            }
            else if (0 != note_to_ascii(note, note_string)) {
                note_string = "\0";
            }
                 
            mvprintw(m, n * 5, "%s", note_string.c_str());
        }
    }
    
    return 0;
}

static int32_t _play_score(
    MidiScore& score,
    MidiOut& output)
{
    string note_string = "\0";
    int32_t rows = 0;
    int32_t cols = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint32_t delay = 0;
    
    getmaxyx(stdscr, rows, cols);
    index = 0;
    
    score.get_bpm(&delay);
    delay = (uint32_t) ((60.0 / (double) delay) * 1e9);

    for (int32_t m = 0; (m < (rows-1)) && (true != score.is_end(index)); m++) {
        
        for (int32_t n = 0; (n < 8) && (true != score.is_end(index)); n++) {

            if (0 != score.get_note(index++, &note)) {
                return -1;
            }
            else if (0 != note_to_ascii(note, note_string)) {
                return -1;
            }

            attron(A_REVERSE | A_BLINK);
            mvprintw(m, n * 5, "%s", note_string.c_str());
            attroff(A_REVERSE | A_BLINK);
            refresh();
            output.note_on(note, 100);
            delay_ns(delay);
            output.note_off(note, 100);
            mvprintw(m, n * 5, "%s", note_string.c_str());
            refresh();
        }
    }
    
    return 0;
}

int main(
    int argc,
    char* argv[])
{
    MidiScore score;
    MidiOut output;
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t input = 0;
    string note_string = "     ";
    uint32_t index = 0;
    uint8_t note = 0;
    uint32_t bpm = 0;
    bool is_exit_requested = false;
    bool is_play_requested = false;
    
    if (!argv[1]) {
        printf("Insufficient magical undocumented parameters\n\
                Better luck next time!\n");
        return -1;
    } 
    else if (0 != output.open(argv[1])) {
        return -1;
    }
    else if (0 != _init_display()) {
        return -1;
    }
    
    score.get_bpm(&bpm);
    
    while (false == is_exit_requested) {
        
        getmaxyx(stdscr, rows, cols);
        werase(stdscr);
        
        PRINT_BPM(rows, cols, bpm);
        _print_score(score);
        
        if (true == is_play_requested) {
            _play_score(score, output);
        }
        
        wmove(stdscr, rows-1, cols-5);
        refresh();
        is_play_requested = false;
        
        while (1) {
            input = getch();
        
            if ('q' == input) {
                is_exit_requested = true;
                break;
            }
            if ('p' == input) {
                is_play_requested = true;
                break;
            }
            else if (KEY_DC == input) {
                if (index > 0) index--;
                score.clear_note(index);
                break;
            }
            else if (KEY_BACKSPACE == input) {
                if (0 < note_string.length()) {
                    note_string.pop_back();
                    mvwaddch(stdscr, rows-1, cols-5+note_string.length(), ' ');
                }
            }
            else if (KEY_UP == input) {
                if (0 == score.set_bpm(bpm + 1)) {
                    PRINT_BPM(rows, cols, ++bpm);
                }
            }
            else if (KEY_LEFT == input) {
                //if (index != 0) --index;
                //_run_note(&score, &output, false, index);
            }
            else if (KEY_RIGHT == input) {
                //_run_note(&score, &output, false, ++index);
            }
            else if (KEY_DOWN == input) {
                if (bpm > 1) {
                    if (0 == score.set_bpm(bpm - 1)) {
                        PRINT_BPM(rows, cols, --bpm);
                    }
                }
            }
            else if ((KEY_ENTER == input) || (10 == input)) {
                if (0 == ascii_to_note(note_string, note)) {
                    score.set_note(index++, note);
                }
                note_string = "\0";
                break;
            }
            else if (isprint(input)) {
                if (4 > note_string.length()) {
                    if (isalpha(input)) {
                        input &= ~0x20;
                    }
                    mvwaddch(stdscr, rows-1, cols-5+note_string.length(), input);
                    note_string += input;                    
                }
            }
        }
    }
    
    endwin();
    
    return 0;
}
