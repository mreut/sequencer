
/***** Includes *****/

#include "ApplicationManager.hpp"
#include "utility.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 15
#define SPACES_PER_NOTE 10
#define SPACES_PER_DIALOG 32

#define MAX_COLUMN ((uint32_t) (this->ui_.get_cols() - SPACES_PER_NOTE))
#define MAX_ROW ((uint32_t) (this->ui_.get_rows()))

#define DISPLAY_INFO_START_ROW 0
#define DISPLAY_INFO_END_ROW 2

#define DISPLAY_FRAME_TOP_ROW (3)
#define DISPLAY_FRAME_BOTTOM_ROW (MAX_ROW - 2)

#define DISPLAY_SCORE_START_ROW (4)
#define DISPLAY_SCORE_END_ROW (MAX_ROW - 3)

#define DISPLAY_COMMAND_LINE_ROW (MAX_ROW)

#define CURRENT_SCORE (this->comp_.get_score())


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
    uint8_t iter = 0;
    uint64_t delay = 0;

    this->mutex_.lock();
    this->play_count = 0;
    this->mutex_.unlock();
    
    this->display_refresh_frame();
    
    while (this->is_play_) {
        max_col = MAX_COLUMN;
        max_row = MAX_ROW;
        // grab the next note
        bpm = p_score->get_bpm();
        note = p_score->get_note(index);
        count = p_score->get_count(index);
        type = p_score->get_count_type(index);
        repeat = p_score->get_repeat();
        index++;
        
        note_count_to_ascii(note, type, count, str);
        
        delay = (uint32_t) ((60.0 / (double) bpm) * 1e9);
        delay = (COUNT_DIVIDE == type) ? delay / count : delay * count;
        
        this->ui_.print(row, col, A_REVERSE | A_BLINK, str);
        if (MIDI_NOTE_REST != note) this->out_.note_on(note, 100);
        delay_ns(delay);
        if (MIDI_NOTE_REST != note) this->out_.note_off(note, 100);
        this->ui_.print(row, col, A_NORMAL, str);
        
        if (true == p_score->is_end(index)) {
            // TODO: next score?
            // TODO: iter?
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
}


/***** Public Class Methods *****/

ApplicationManager::ApplicationManager(
    void)
{
    this->is_play_ = false;
    this->command_line_ = "";
    this->index_ = 0;
    this->origin_ = 0;
    this->play_count = 0;
}

ApplicationManager::~ApplicationManager(
    void)
{
    if (this->is_play_) {
        this->is_play_ = false;
        this->play_thread_.join();
    }
}

void ApplicationManager::display_refresh(
    void)
{
    this->display_refresh_frame();
    this->display_refresh_info();
    this->display_refresh_current_score();
}

void ApplicationManager::display_refresh_info(
    void)
{
    MidiScore* const p_score = CURRENT_SCORE;
    string str = "";
    string tmp = "";
    bool is_play = false;

    this->mutex_.lock();
    
    str = (this->is_play_) ? "[*] " : "[ ] ";
    str += "Score";
    this->ui_.print(DISPLAY_INFO_START_ROW, 0, A_NORMAL, str);
    
    // 15 spaces per param.
    str = "Repeat         BPM            Origin         Index";
    this->ui_.print(DISPLAY_INFO_START_ROW + 1, 0, A_NORMAL, str);
    
    str = "";
    // Repeat
    tmp = "0/" + to_string(p_score->get_repeat());
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    
    // BPM
    tmp = to_string(p_score->get_bpm());
    str += tmp + string(SPACES_PER_PARAM - tmp.length(), ' ');
    
    // TODO: Origin
    tmp = "0";
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
    uint32_t index = 0;
    enum count_type type;
    string str = "";
    string line = "";
    uint8_t note = 0;
    uint8_t count = 0;
    
    this->mutex_.lock();
    index = this->origin_;
    this->mutex_.unlock();
    
    while (row <= DISPLAY_SCORE_END_ROW) {
        if (true != p_score->is_end(index)) {
            // grab the next note
            note = p_score->get_note(index);
            count = p_score->get_count(index);
            type = p_score->get_count_type(index);
            index++;
            
            note_count_to_ascii(note, type, count, str);
            
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
    
    // TODO: command line
    string str = "Hello World!";
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

void ApplicationManager::echo_command_line(
    application_command command,
    string entry)
{
    this->mutex_.lock();
    
    switch (command) {
    case (CMD_BPM):
        this->command_line_ = "[BPM]: " + entry;
        break;

    case (CMD_INDEX):
        this->command_line_ = "[INDEX]: " + entry;
        break;
    
    case (CMD_ORIGIN):
        this->command_line_ = "[ORIGIN]: " + entry;
        break;
    
    case (CMD_REPEAT):
        this->command_line_ = "[REPEAT]: " + entry;
        break;
    
    case (CMD_NOTE):
        this->command_line_ = "[NOTE]: " + entry;
        break;
    
    case (CMD_SAVE):
        this->command_line_ = "[SAVE]: " + entry;
        break;
    
    case (CMD_LOAD):
        this->command_line_ = "[LOAD]: " + entry;
        break;
    
    case (CMD_BPM_DECREMENT):
    case (CMD_BPM_INCREMENT):
    case (CMD_INDEX_DECREMENT):
    case (CMD_INDEX_INCREMENT):
    case (CMD_DELETE):
    case (CMD_PLAY):
    case (CMD_QUIT):
    case (CMD_INVALID):
        this->command_line_ = ": " + entry;
        break;
    }
    
    this->mutex_.unlock();
    this->display_refresh_command_line();
}

void ApplicationManager::enter_command_line(
    application_command command,
    string entry="N/A")
{
    MidiScore* const p_score = CURRENT_SCORE;
    enum count_type type;
    uint8_t note;
    uint8_t count;
    uint16_t bpm;
    bool refresh_score = false;
    bool refresh_info = false;
    
    this->mutex_.lock();
    
    switch (command) {
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
            this->mutex_.lock();
            this->index_ = stoi(entry);
            this->mutex_.unlock();
            refresh_info = true;
        }
        break;
    
    case (CMD_INDEX_DECREMENT):
        this->mutex_.lock();
        if (0 != this->index_) this->index_--;
        this->mutex_.unlock();
        refresh_info = true;
        break;
    
    case (CMD_INDEX_INCREMENT):
        this->mutex_.lock();
        this->index_++;
        this->mutex_.unlock();
        refresh_info = true;
        break;
    
    case (CMD_ORIGIN):
        if (is_number(entry)) {
        //    app.display = stoi(entry);
        //    app.index = app.display;
        //    app.is_ui_refresh = true;
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
        if (this->is_play_) {
            this->is_play_ = false;
            this->play_thread_.join();
            this->display_refresh();
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
    
    this->command_line_ = "";
    
    this->mutex_.unlock();
    
    if (refresh_info) this->display_refresh_info();
    if (refresh_score) this->display_refresh_current_score();
    this->display_refresh_command_line();
}
