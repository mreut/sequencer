
/***** Includes *****/

#include "ApplicationManager.hpp"
#include "utility.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 15
#define SPACES_PER_NOTE 12
#define SPACES_PER_DIALOG 32

#define MAX_COLUMN ((uint32_t) (this->ui_.get_cols() - SPACES_PER_NOTE))
#define MAX_ROW ((uint32_t) (this->ui_.get_rows()) - 1)

#define DISPLAY_INFO_START_ROW 0
#define DISPLAY_INFO_END_ROW 2

#define DISPLAY_FRAME_TOP_ROW (3)
#define DISPLAY_FRAME_BOTTOM_ROW (MAX_ROW - 1)

#define DISPLAY_SCORE_START_ROW (4)
#define DISPLAY_SCORE_END_ROW (MAX_ROW - 3)

#define DISPLAY_COMMAND_LINE_ROW (MAX_ROW)

#define COMMAND_LINE_DEFAULT ("[#]")

#define CURRENT_SCORE (this->comp_.get_score())

#define MULTICAST_IP ("225.0.0.37")
#define MULTICAST_PORT (12345)


/***** Private Class Methods *****/

void ApplicationManager::play_main(
    void)
{
    MidiScore* p_score = CURRENT_SCORE;
    uint32_t max_col = MAX_COLUMN;
    uint32_t max_row = MAX_ROW;
    enum count_type type;
    string str = "";
    uint32_t row = DISPLAY_SCORE_START_ROW;
    uint32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    uint16_t bpm = 0;
    uint8_t repeat = 0;
    uint64_t delay = 0;

    this->mutex_.lock();
    this->play_count_ = 0;
    this->mutex_.unlock();
    
    this->display_refresh_info();
    
    while (this->is_play_) {
        max_col = MAX_COLUMN - SPACES_PER_NOTE;
        max_row = MAX_ROW;
        // grab the next note
        bpm = p_score->get_bpm();
        note = p_score->get_note(index);
        count = p_score->get_count(index);
        type = p_score->get_count_type(index);
        repeat = p_score->get_repeat();        
        note_count_to_ascii(note, type, count, str);
        
        this->mutex_.lock();
        if (index++ == this->index_) str = "[" + str + "]";
        this->mutex_.unlock();
        
        delay = (uint32_t) ((60.0 / (double) bpm) * 1e9);
        delay = (COUNT_DIVIDE == type) ? delay / count : delay * count;
        
        this->ui_.print(row, col, A_REVERSE | A_BLINK, str);
        if (MIDI_NOTE_REST != note) this->out_.note_on(note, 100);
        delay_ns(delay);
        if (MIDI_NOTE_REST != note) this->out_.note_off(note, 100);
        this->ui_.print(row, col, A_NORMAL, str);
        
        if (true == p_score->is_end(index)) {
            
            this->mutex_.lock();
            this->play_count_ += 1;
            if (repeat < this->play_count_) {
                this->play_count_ = 0;
                this->comp_.next_score();
                p_score = CURRENT_SCORE;
            }
            this->mutex_.unlock();
            
            this->display_refresh();
            index = 0;
            col = 0;
            row = DISPLAY_SCORE_START_ROW;
        }
        else if (col >= max_col) {
            if (row >= max_row) {
                // TODO: set play display?
                this->display_refresh_current_score();
                col = 0;
                row = DISPLAY_SCORE_START_ROW;
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
    
    this->mutex_.lock();
    this->play_count_ = 0;
    this->mutex_.unlock();
}

void ApplicationManager::slave_main(
    void)
{
    enum application_command cmd = CMD_INVALID;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    
    while (this->is_running_) {
        if (this->socket_.slave_recv((uint8_t*) &cmd, sizeof(cmd), timeout)) {
            
            if (CMD_PLAY == cmd) {
                this->mutex_.lock();
                if (this->is_play_) {
                    this->is_play_ = false;
                    this->mutex_.unlock();
                    this->play_thread_.join();
                    this->display_refresh();
                }
                else {
                    this->is_play_ = true;
                    this->mutex_.unlock();
                    this->play_thread_ = thread(&ApplicationManager::play_main,
                                                this);
                    this->display_refresh_info();
                }
            }
        }
        cmd = CMD_INVALID;
    }
}

void ApplicationManager::display_refresh(
    void)
{
    this->display_refresh_frame();
    this->display_refresh_info();
    this->display_refresh_current_score();
    this->display_refresh_command_line();
}

void ApplicationManager::display_refresh_info(
    void)
{
    MidiScore* const p_score = CURRENT_SCORE;
    string str = "";
    string tmp = "";

    this->mutex_.lock();
    
    str = (this->is_play_) ? "[*] " : "[ ] ";
    str += p_score->get_name();
    str += string(MAX_COLUMN - str.length(), ' ');
    this->ui_.print(DISPLAY_INFO_START_ROW, 0, A_NORMAL, str);
    
    // 15 spaces per param.
    str = "Repeat         BPM            Origin         Index";
    this->ui_.print(DISPLAY_INFO_START_ROW + 1, 0, A_NORMAL, str);
    
    str = "";
    // Repeat
    tmp = to_string(this->play_count_) + "/" + to_string(p_score->get_repeat());
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    
    // BPM
    tmp = to_string(p_score->get_bpm());
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    
    // Origin
    tmp = to_string(this->origin_);
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    
    // Index
    tmp = to_string(this->index_);
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    this->ui_.print(DISPLAY_INFO_START_ROW + 2, 0, A_NORMAL, str);
    
    this->mutex_.unlock();
}

void ApplicationManager::display_refresh_frame(
    void)
{
    string str = string(MAX_COLUMN, '=');
    this->ui_.print(DISPLAY_FRAME_TOP_ROW, 0, A_NORMAL, str);
    this->ui_.print(DISPLAY_FRAME_BOTTOM_ROW, 0, A_NORMAL, str);
}

void ApplicationManager::display_refresh_current_score(
    void)
{
    MidiScore* const p_score = CURRENT_SCORE;
    uint32_t row = DISPLAY_SCORE_START_ROW;
    uint32_t n = 0;
    enum count_type type;
    string str = "";
    string line = "";
    uint8_t note = 0;
    uint8_t count = 0;
    
    this->mutex_.lock();
    n = this->origin_;
    this->mutex_.unlock();
    
    while (row <= DISPLAY_SCORE_END_ROW) {
        if (true != p_score->is_end(n)) {
            // grab the next note
            note = p_score->get_note(n);
            count = p_score->get_count(n);
            type = p_score->get_count_type(n);            
            note_count_to_ascii(note, type, count, str);
            
            this->mutex_.lock();
            if (n++ == this->index_) str = "[" + str + "]";
            this->mutex_.unlock();
            
            if (SPACES_PER_NOTE > str.length()) {
                // pad with spaces to fully erase previous
                str += string(SPACES_PER_NOTE - str.length(), ' ');
            }
        }
        else {
            // pad with spaces to fully erase previous
            str = string(SPACES_PER_NOTE, ' ');
        }
        
        line += str;
        
        if (MAX_COLUMN <= line.length()) {
            // print the current row
            this->ui_.print(row, 0, A_NORMAL, line);
            line = "";
            row += 1;
        }
    }
}

void ApplicationManager::display_refresh_command_line(
    void)
{
    this->mutex_.lock();
    string str = this->command_line_;
    str += string(MAX_COLUMN - str.length(), ' ');
    this->ui_.print(DISPLAY_COMMAND_LINE_ROW, 0, A_NORMAL, str);
    this->mutex_.unlock();
}

void ApplicationManager::select_next_score(
    void)
{
    this->comp_.next_score();
    this->display_refresh();
}
    
void ApplicationManager::select_past_score(
    void)
{
    this->comp_.past_score();
    this->display_refresh();
}


/***** Public Class Methods *****/

ApplicationManager::ApplicationManager(
    void)
{
    this->is_running_ = true;
    this->is_play_ = false;
    this->command_ = CMD_INVALID;
    this->command_line_ = COMMAND_LINE_DEFAULT;
    this->index_ = 0;
    this->origin_ = 0;
    this->play_count_ = 0;
}

ApplicationManager::~ApplicationManager(
    void)
{
    this->is_running_ = false;
    
    if (this->is_play_) {
        this->is_play_ = false;
        this->play_thread_.join();
    }
    
    if (this->socket_.is_slave()) {
        this->slave_thread_.join();
    }
}

int32_t ApplicationManager::midi_out_start(
    string port_name)
{
    return this->out_.open(port_name.c_str());
}

int32_t ApplicationManager::display_start(
    void)
{
    this->ui_.start();
    this->display_refresh();
    
    return 0;
}

int32_t ApplicationManager::get_input(
    void)
{
    int32_t ch = 0;
    
    if (0 != this->ui_.get_input(ch)) {
        ch = -1;
    }
    
    return ch;
}

void ApplicationManager::echo_command(
    application_command command,
    string entry)
{
    this->mutex_.lock();
    
    switch (command) {
    case (CMD_CREATE_SCORE):
        this->command_line_ = "[C]reate " + entry;
        break;
        
    case (CMD_TITLE_SCORE):
        this->command_line_ = "[T]itle " + entry;
        break;
        
    case (CMD_BPM):
        this->command_line_ = "[B]PM " + entry;
        break;

    case (CMD_INDEX):
        this->command_line_ = "[I]ndex " + entry;
        break;
    
    case (CMD_ORIGIN):
        this->command_line_ = "[O]rigin " + entry;
        break;
    
    case (CMD_REPEAT):
        this->command_line_ = "[R]epeat " + entry;
        break;
    
    case (CMD_NOTE):
        this->command_line_ = "[N]ote " + entry;
        break;
    
    case (CMD_SAVE):
        this->command_line_ = "[S]ave " + entry;
        break;
    
    case (CMD_LOAD):
        this->command_line_ = "[L]oad " + entry;
        break;
    
    case (CMD_VIEW_NEXT_SCORE):
    case (CMD_BPM_DECREMENT):
    case (CMD_BPM_INCREMENT):
    case (CMD_INDEX_DECREMENT):
    case (CMD_INDEX_INCREMENT):
    case (CMD_DELETE):
    case (CMD_PLAY):
    case (CMD_QUIT):
    case (CMD_INVALID):
        this->command_line_ = "[ ]";
        break;
    }
    
    this->mutex_.unlock();
    this->display_refresh_command_line();
}

void ApplicationManager::enter_command(
    application_command command,
    string entry)
{
    MidiScore* p_score = CURRENT_SCORE;
    enum count_type type;
    uint8_t note;
    uint8_t count;
    uint16_t bpm;
    bool refresh_score = false;
    bool refresh_info = false;
    bool end_play = false;
    
    this->mutex_.lock();
    
    switch (command) {
    case (CMD_CREATE_SCORE):
        this->comp_.create_next_score();
        this->comp_.next_score();
        this->origin_ = 0;
        this->index_ = 0;
        p_score = CURRENT_SCORE;
        p_score->set_name(entry);
        refresh_score = true;
        refresh_info = true;
        break;
        
    case (CMD_TITLE_SCORE):
        p_score->set_name(entry);
        refresh_info = true;
        break;
        
    case (CMD_VIEW_NEXT_SCORE):
        this->comp_.next_score();
        refresh_score = true;
        refresh_info = true;
        break;
        
    case (CMD_BPM):
        if (is_number(entry)) {
            bpm = stoi(entry);
            p_score->set_bpm(bpm);
            refresh_info = true;
        }
        break;
    
    case (CMD_BPM_DECREMENT):
        bpm = p_score->get_bpm();
        if (bpm > 1) bpm--;
        p_score->set_bpm(bpm);
        refresh_info = true;
        break;
        
    case (CMD_BPM_INCREMENT):
        bpm = p_score->get_bpm();
        if (bpm < UINT16_MAX) bpm++;
        p_score->set_bpm(bpm);
        refresh_info = true;
        break;
    
    case (CMD_INDEX):
        if (is_number(entry)) {
            this->index_ = stoi(entry);
            refresh_info = true;
            refresh_score = true;
        }
        break;
    
    case (CMD_INDEX_DECREMENT):
        if (0 != this->index_) this->index_--;
        refresh_info = true;
        refresh_score = true;
        break;
    
    case (CMD_INDEX_INCREMENT):
        this->index_++;
        refresh_info = true;
        refresh_score = true;
        break;
    
    case (CMD_ORIGIN):
        if (is_number(entry)) {
            this->origin_ = stoi(entry);
            this->index_ = this->origin_;
            refresh_info = true;
            refresh_score = true;
        }
        refresh_info = true;
        refresh_score = true;
        break;
    
    case (CMD_REPEAT):
        if (is_number(entry)) {
            p_score->set_repeat(stoi(entry));
            this->display_refresh_frame();
            refresh_info = true;
        }
        break;
    
    case (CMD_NOTE):
        if (ascii_to_note_count(entry, note, type, count)) {
            p_score->set_note(this->index_, note);
            p_score->set_count(this->index_, count);
            p_score->set_count_type(this->index_, type);
            this->index_++;
            this->display_refresh_info();
            this->display_refresh_current_score();
            refresh_score = true;
        }
        break;
    
    case (CMD_SAVE):
        p_score->save(entry);
        break;
    
    case (CMD_LOAD):
        p_score->load(entry);
        refresh_score = true;
        refresh_info = true;
        break;
    
    case (CMD_DELETE):
        refresh_score = true;
        refresh_info = true;
        break;
        
    case (CMD_PLAY):
        if (this->socket_.is_master()) {
            this->socket_.master_send((uint8_t*) &command, sizeof(command));
        }
    
        if (this->is_play_) {
            this->is_play_ = false;
            end_play = true;
            refresh_info = true;
            refresh_score = true;
        }
        else {
            this->is_play_ = true;
            this->play_thread_ = thread(&ApplicationManager::play_main, this);
        }
        break;
        
    case (CMD_QUIT):
    case (CMD_INVALID):
        // do nothing
        break;
    }
    
    this->command_line_ = COMMAND_LINE_DEFAULT;
    
    this->mutex_.unlock();
    
    if (end_play) this->play_thread_.join();
    if (refresh_info) this->display_refresh_info();
    if (refresh_score) this->display_refresh_current_score();
    this->display_refresh_command_line();
}

void ApplicationManager::start_master(
    void)
{
    if ((this->socket_.is_master()) || (this->socket_.is_slave())) {
        return;
    }
    
    this->socket_.master_open(MULTICAST_IP, MULTICAST_PORT);
}
    
void ApplicationManager::start_slave(
    void)
{
    if ((this->socket_.is_master()) || (this->socket_.is_slave())) {
        return;
    }
    
    this->socket_.slave_open(MULTICAST_IP, MULTICAST_PORT);
    this->slave_thread_ = thread(&ApplicationManager::slave_main, this);
}
