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
            app.echo_command(cmd, entry);
        }
        else {
            cmd = CMD_INVALID;
            app.echo_command(cmd);
        }
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
    case (CMD_SAVE):
    case (CMD_LOAD):
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
        app.echo_command(cmd);
    }
    else if (CMD_INDEX == cmd) {
        app.enter_command(CMD_INDEX_DECREMENT);
        app.echo_command(cmd);
    }
    
}

static void _handle_right(
    ApplicationManager& app,
    enum application_command& cmd)
{
    if (CMD_BPM == cmd) {
        app.enter_command(CMD_BPM_INCREMENT);
        app.echo_command(cmd);
    }
    else if (CMD_INDEX == cmd) {
        app.enter_command(CMD_INDEX_INCREMENT);
        app.echo_command(cmd);
    }
}

static void _handle_character(
    ApplicationManager& app,
    enum application_command& cmd,
    string& entry,
    int32_t ch)
{
    if (CMD_INVALID == cmd) {
        // capitalize all alphabetical input for command character
        if (isalpha(ch)) ch &= ~0x20;
        
        switch (ch) {
        case (CMD_CREATE_SCORE):
        case (CMD_TITLE_SCORE):
        case (CMD_BPM):
        case (CMD_INDEX):
        case (CMD_NOTE):
        case (CMD_ORIGIN):
        case (CMD_REPEAT):
        case (CMD_SAVE):
        case (CMD_LOAD):
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
        case (CMD_NOTE):
        case (CMD_BPM):
        case (CMD_INDEX):
        case (CMD_ORIGIN):
        case (CMD_REPEAT):
        case (CMD_SAVE):
        case (CMD_LOAD):
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

    if (argv[2]) {
        if ('m' == argv[2][0]) {
            app.start_master();
        }
        else if ('s' == argv[2][0]) {
            app.start_slave();
        }
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
