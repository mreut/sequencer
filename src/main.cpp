/***** Includes *****/

#include "ApplicationManager.hpp"


/***** Local Functions *****/

static void _handle_backspace(
    enum application_command& cmd,
    string& entry)
{
    if (0 < entry.length()) {
        entry.pop_back();
    }
    else {
        cmd = CMD_INVALID;
    }
}

static void _handle_enter(
    enum application_command& cmd,
    string& entry)
{
    cmd = CMD_INVALID;
    entry = "";
}

static void _handle_left(
    enum application_command& cmd)
{
    return;
}

static void _handle_right(
    enum application_command& cmd)
{
    return;
}

static void _handle_character(
    enum application_command& cmd,
    string& entry,
    int32_t ch)
{
#if 0
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
#endif
}

/***** Global Functions *****/

int main(
    int argc,
    char* argv[])
{
    ApplicationManager app;
    enum application_command cmd = CMD_INVALID;
    string entry = "";
    int32_t in = 0;
    int32_t r = -1;
    
    if (!argv[1]) {
        printf("Error: missing sequencer file name\n");
        goto main_exit;
    }
    else if (0 != app.midi_out_start(argv[1])) {
        printf("Error: failed to open midi output device\n");
        goto main_exit;
    }

    app.display_start();
    
    sleep(4);
    
#if 0
    while (1) {

        if (0 != app.ui.get_input(in)) {
        if (0 != app.ui.get_input(in)) {
            break;
        }
        else if (isprint(in)) {
            _handle_character(cmd, entry, in);
        }
        else if (KEY_BACKSPACE == in) {
            _handle_backspace(cmd, entry);
        }
        else if ((KEY_ENTER == in) || (10 == in)) {
            _handle_enter(cmd, entry);
        }
        else if (KEY_LEFT == in) {
            _handle_left(cmd);
        }
        else if (KEY_RIGHT == in) {
            _handle_right(cmd);
        }
    }
#endif
    r = 0;

main_exit:
    return r;
}
