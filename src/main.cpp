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

static bool _arg_parse(
    ApplicationManager& app,
    int argc,
    char* argv[])
{
    string s;
    bool is_midi_open = false;
    bool success = true;
    
    for (int n = 1; (n < argc) && (success); n++) {
        s = string(argv[n]);
        
        if ((0 == s.compare("--midi")) ||
            (0 == s.compare("-m"))) {
            // midi output device name
            if ((n++ + 1) < argc) {
                if ((!is_midi_open) && (0 != app.midi_out_start(argv[n]))) {
                    // failed to open midi output
                    success = false;
                }
                else {
                    // TODO: subsequent midi out will not be reported as error
                    is_midi_open = true;
                }
            }
            else {
                // no argument provided
                success = false;
            }
        }
        else if ((0 == s.compare("--master")) ||
                 (0 == s.compare("-m"))) {
            
            if (!app.is_slave()) {
                app.start_master();
            }
            else {
                // already opened as slave
                success = false;
            }
        }
        else if ((0 == s.compare("--slave")) ||
                 (0 == s.compare("-s"))) {
            
            if (!app.is_master()) {
                app.start_slave();
            }
            else {
                // already opened as master
                success = false;
            }
        }
        else {
            // unrecognized option
            success = false;
        }
    }
    
    if (!is_midi_open) {
        // TODO: add function in ApplicationManager to better track this...
        // failed to specify midi output
        success = false;
    }
    
    return success;
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
    
    if (!_arg_parse(app, argc, argv)) {
        printf("Error: bad command line argument\n");
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
