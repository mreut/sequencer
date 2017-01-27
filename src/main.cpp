/***** Includes *****/

#include "ApplicationManager.hpp"


/***** Local Variables *****/

bool _is_running = true;


/***** Local Functions *****/

static void _handle_backspace(
    ApplicationManager& app,
    enum application_command& cmd,
    string& entry)
{
    if (CMD_INVALID != cmd) {
        if (0 < entry.length()) {
            entry.pop_back();
        }
        else {
            cmd = CMD_INVALID;
        }
        app.echo_command(cmd, entry);
    }
}

static void _handle_enter(
    ApplicationManager& app,
    enum application_command& cmd,
    string& entry)
{
    switch (cmd) {
    case (CMD_CREATE_SCORE):
    case (CMD_TITLE_SCORE):
    case (CMD_BPM):
    case (CMD_INDEX):
    case (CMD_NOTE):
    case (CMD_ORIGIN):
    case (CMD_REPEAT):
        app.enter_command(cmd, entry);
        cmd = CMD_INVALID;
        entry = "";
        break;

    case (CMD_BPM_DECREMENT):
    case (CMD_BPM_INCREMENT):
    case (CMD_INDEX_DECREMENT):
    case (CMD_INDEX_INCREMENT):
    case (CMD_INVALID):
    default:
        cmd = CMD_INVALID;
        entry = "";
    }
}

static void _handle_left(
    ApplicationManager& app,
    enum application_command& cmd)
{
    if (CMD_BPM == cmd) {
        app.enter_command(CMD_BPM_DECREMENT);
    }
    else if (CMD_INDEX) {
        app.enter_command(CMD_INDEX_DECREMENT);
    }
}

static void _handle_right(
    ApplicationManager& app,
    enum application_command& cmd)
{
    if (CMD_BPM == cmd) {
        app.enter_command(CMD_BPM_INCREMENT);
    }
    else if (CMD_INDEX) {
        app.enter_command(CMD_INDEX_INCREMENT);
    }
}

static void _handle_character(
    ApplicationManager& app,
    enum application_command& cmd,
    string& entry,
    int32_t ch)
{
    if (CMD_INVALID == cmd) {
        switch (ch) {
        case (CMD_CREATE_SCORE):
        case (CMD_TITLE_SCORE):
        case (CMD_BPM):
        case (CMD_INDEX):
        case (CMD_NOTE):
        case (CMD_ORIGIN):
        case (CMD_REPEAT):
            cmd = (enum application_command) ch;
            entry = "";
            app.echo_command(cmd, entry);
            break;
        
        case (CMD_VIEW_NEXT_SCORE):
            app.enter_command(CMD_VIEW_NEXT_SCORE);
            break;
            
        case (CMD_PLAY):
            app.enter_command(CMD_PLAY);
            break;
        
        case (CMD_DELETE):
            app.enter_command(CMD_DELETE);
            break;
        
        case (CMD_QUIT):
            _is_running = false;
            break;
        
        default:
            cmd = CMD_INVALID;
            entry = "";
        }
    }
    else {
        switch (cmd) {
        case (CMD_CREATE_SCORE):
        case (CMD_TITLE_SCORE):
        case (CMD_BPM):
        case (CMD_INDEX):
        case (CMD_NOTE):
        case (CMD_ORIGIN):
        case (CMD_REPEAT):
            entry += ch;
            app.echo_command(cmd, entry);
            break;

        case (CMD_VIEW_NEXT_SCORE):
        case (CMD_BPM_DECREMENT):
        case (CMD_BPM_INCREMENT):
        case (CMD_INDEX_DECREMENT):
        case (CMD_INDEX_INCREMENT):
        case (CMD_INVALID):
        default:
            cmd = CMD_INVALID;
            entry = "";
            app.echo_command(cmd, entry);
        }
    }
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
    
    while (_is_running) {

        in = app.get_input();
        if (in < 0) {
            _is_running = false;
        }
        else if (isprint(in)) {
            _handle_character(app, cmd, entry, in);
        }
        else if (KEY_BACKSPACE == in) {
            _handle_backspace(app, cmd, entry);
        }
        else if ((KEY_ENTER == in) || (10 == in)) {
            _handle_enter(app, cmd, entry);
        }
        else if (KEY_LEFT == in) {
            _handle_left(app, cmd);
        }
        else if (KEY_RIGHT == in) {
            _handle_right(app, cmd);
        }
    }
    
    r = 0;

main_exit:
    return r;
}
