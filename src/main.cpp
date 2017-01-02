#include <sys/ioctl.h>
#include <ncurses.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cerrno>
#include <time.h>

#include "MidiScore.hpp"

#if 0
static int32_t delay_ns(
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
#endif

int32_t _init_display(
    void)
{
    initscr();
    raw();  /* Line buffering disabled */
    keypad(stdscr, TRUE);   /* We get F1, F2 etc.. */
    noecho();
    refresh();
    curs_set(0);
    return 0;
}

int32_t _ascii_to_note(
    uint8_t* p_string,
    uint8_t* p_note)
{
    uint8_t note = 0;
    uint8_t octave = 0;
    
    switch(p_string[0]) {
    case ('A'):
        note = 9;
        break;
    case ('B'):
        note = 11;
        break;
    case ('C'):
        note = 0;
        break;
    case ('D'):
        note = 2;
        break;
    case ('E'):
        note = 4;
        break;
    case ('F'):
        note = 5;
        break;
    case ('G'):
        note = 7;
        break;
    default:
        return -1;
    }
    
    if (isdigit(p_string[1])) {
        if (isdigit(p_string[2])) {
            octave = 10;
        }
        else {
            octave = p_string[1] - '0';
        }
    }
    else {
        return -1;
    }
    
    note += 12 * octave;
    
    if (('#' == p_string[2]) || ('#' == p_string[3])) {
        note += 1;
    }
    
    if (note > MIDI_NOTE_MAX) {
        return -1;
    }
    
    *p_note = note;
    
    return 0;
}

int32_t _note_to_ascii(
    uint8_t note,
    uint8_t* p_string)
{
    int32_t octave = 0;
    uint8_t mod = ' ';

    if (note > MIDI_NOTE_MAX) {
        return -1;
    }
    
    while (note >= 12) {
        octave++;
        note -= 12;
    }
    
    switch (note) {
    case (0):
        note = 'C';
        break;
        
    case (1):
        note = 'C';
        mod = '#';
        break;
    
    case (2):
        note = 'D';
        break;
        
    case (3):
        note = 'D';
        mod = '#';
        break;
        
    case (4):
        note = 'E';
        break;
        
    case (5):
        note = 'F';
        break;
        
    case (6):
        note = 'F';
        mod = '#';
        break;
        
    case (7):
        note = 'G';
        break;
        
    case (8):
        note = 'G';
        mod = '#';
        break;
        
    case (9):
        note = 'A';
        break;
        
    case (10):
        note = 'A';
        mod = '#';
        break;
        
    case (11):
        note = 'B';
        break;
    }
    
    if (octave < 10) {
        p_string[0] = note;
        p_string[1] = '0' + octave;
        p_string[2] = mod;
        p_string[3] = ' ';
        p_string[4] = '\0';
    }
    else {
        p_string[0] = note;
        p_string[1] = '1';
        p_string[2] = '0';
        p_string[0] = note;
        p_string[5] = '\0';
    }
    
    return 0;
}

int main(
    void)
{
    MidiScore score;
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t input = 0;
    int32_t input_count = 0;
    uint8_t p_note_string[5] = {' ', ' ', ' ', ' ', '\0'};
    uint32_t index = 0;
    uint8_t note = 0;
    bool is_exit_requested = false;
    
    _init_display();
    
    while (false == is_exit_requested) {
        
        getmaxyx(stdscr, rows, cols);
        index = 0;
        
        for (int m = 0; m < (rows-1); m++) {
            
            for (int n = 0; n < 8; n++) {
                
                if (true != score.is_end(index)) {
                    score.get_note(index++, &note);
                    if (0 != _note_to_ascii(note, p_note_string)) {
                        p_note_string[0] = ' ';
                        p_note_string[1] = ' ';
                        p_note_string[2] = ' ';
                        p_note_string[3] = ' ';
                        p_note_string[4] = '\0';
                    }
                }
                else {
                    p_note_string[0] = ' ';
                    p_note_string[1] = ' ';
                    p_note_string[2] = ' ';
                    p_note_string[3] = ' ';
                    p_note_string[4] = '\0';
                }
                
                mvprintw(m, n * 5, "%s", p_note_string);
                
            }
        }
        
        wmove(stdscr, rows-1, cols-5);
        refresh();
        input_count = 0;
        
        while (1) {
            input = getch();
        
            if ('q' == input) {
                is_exit_requested = true;
                break;
            }
            else if (KEY_DC == input) {
                score.clear_note(--index);
                break;
            }
            else if (KEY_BACKSPACE == input) {
                if (0 < input_count) {
                    p_note_string[--input_count] = ' ';
                    mvwaddch(stdscr, rows-1, cols-5+input_count, ' ');
                }
            }
            else if ((KEY_ENTER == input) || (10 == input)) {
                if (0 == _ascii_to_note(p_note_string, &note)) {
                    score.set_note(index, note);
                }
                werase(stdscr);
                break;
            }
            else if (isprint(input)) {
                if (4 > input_count) {
                    if (isalpha(input)) {
                        input &= ~0x20;
                    }
                    mvwaddch(stdscr, rows-1, cols-5+input_count, input);
                    p_note_string[input_count++] = input;                    
                }
            }
        }
    }
    
    endwin();
    
    return 0;
}
