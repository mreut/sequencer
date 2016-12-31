#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include <time.h>

#include "MidiScore.hpp"

#if 0
static int32_t hal_nanosleep(
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

static void advance_cursor(
    void)
{
    static const char cursor[4] = {'/', '-', '\\', '|'};
    static int pos = 0;
    fprintf(stderr, "%c\b", cursor[pos]);
    pos = (pos + 1) % 4;
}
#endif

static void _get_display_dimentions(
    int32_t* p_rows,
    int32_t* p_cols)
{
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);

    *p_rows = w.ws_row;
    *p_cols = w.ws_col;
}

uint8_t mygetch(
    void)
{
    struct termios oldt, newt;
    uint8_t ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

int main (void)
{
    MidiScore score;
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t col_offset = 0;
    uint8_t input = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    bool is_exit_resquested = false;
    
    _get_display_dimentions(&rows, &cols);

    col_offset = cols / 2 - 32;
    
    while (false == is_exit_resquested) {
        
        for (int m = 0; true != score.is_end(m); m++) {
            if (0 == (m % 16)) {
                for (int n = 0; n < col_offset; n++) {
                    fprintf(stdout, " ");
                }
            }
            
            score.get_note(m, &note);
            fflush(stdout);
            fprintf(stdout, "%c   ", note);
            
            if (0 == ((m+1) % 16)) {
                fprintf(stdout, "\n\n");
            }
        }
        
        fprintf(stdout, "\r\n\r\n\r\n");
        
        input = mygetch();
        if ('x' == input) {
            is_exit_resquested = true;
        }
        //fprintf(stdout, "%c\r\n", input);
        score.set_note(index++, input);
    }
    
    return 0;
}
