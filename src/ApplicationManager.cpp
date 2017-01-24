
/***** Includes *****/

#include "ApplicationManager.hpp"
#include "utility.hpp"


/***** Defines *****/

#define SPACES_PER_PARAM 10
#define SPACES_PER_NOTE 10
#define SPACES_PER_DIALOG 32
#define DISPLAY_START_ROW 2

#define CURRENT_SCORE (this->comp_.get_score())
#define CURRENT_INDEX (this->get_index())
#define MAX_COLUMN (this->ui_.get_cols() - SPACES_PER_NOTE)
#define MAX_ROW (this->ui_.get_rows() - 3)


/***** Private Class Methods *****/

void ApplicationManager::play_main(
    void)
{
    MidiScore* p_score = CURRENT_SCORE;
    uint32_t max_col = MAX_COLUMN;
    uint32_t max_row = MAX_ROW;
    enum count_type type;
    string str = "";
    uint32_t row = DISPLAY_START_ROW;
    uint32_t col = 0;
    uint32_t index = 0;
    uint8_t note = 0;
    uint8_t count = 0;
    uint16_t bpm = 0;
    uint8_t repeat = 0;
    uint8_t iter = 0;
    uint64_t delay = 0;

    this->display_refresh_frame();
    
    while (this->is_play_) {
        p_score->get_bpm(bpm);
        p_score->get_repeat(repeat);
        max_col = MAX_COLUMN;
        max_row = MAX_ROW;
        
        if ((0 != p_score->get_note_count(index++, note, type, count)) || 
            (!note_count_to_ascii(note, type, count, str))) {
            return;
        }
        
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
            row = DISPLAY_START_ROW;
        }
        else if (col >= max_col) {
            if (row >= max_row) {
                // TODO: set play display?
                this->display_refresh_current_score();
                col = 0;
                row = DISPLAY_START_ROW;
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

uint32_t ApplicationManager::get_index(
    void)
{
    uint32_t index = 0;
    
    this->mutex_.lock();
    index = this->index_;
    this->mutex_.unlock();
    
    return index;
}
    
uint32_t ApplicationManager::get_display(
    void)
{
    uint32_t display = 0;
    
    return display;
}


/***** Public Class Methods *****/

void ApplicationManager::display_start(
    void)
{
    this->ui_.start();
}

void ApplicationManager::display_refresh(
    void)
{
    this->display_refresh_frame();
    this->display_refresh_current_score();
}

void ApplicationManager::display_refresh_frame(
    void)
{
    MidiScore* const p_score = CURRENT_SCORE;
    const uint32_t max_line_len = MAX_COLUMN;
    const uint32_t max_row = MAX_ROW;
    string str = "";
    uint8_t tmp = 0;
    
    p_score->get_repeat(tmp);
    str = "0:" + to_string(tmp) + " ";
    this->ui_.print(0, 0, A_NORMAL, str);
    str = string(max_line_len, '=');
    this->ui_.print(1, 0, A_NORMAL, str);
    this->ui_.print(max_row + 1, 0, A_NORMAL, str);
}

void ApplicationManager::display_refresh_current_score(
    void)
{
    MidiScore* const p_score = CURRENT_SCORE;
    const uint32_t max_line_len = MAX_COLUMN;
    const uint32_t max_row = MAX_ROW;
    uint32_t row = DISPLAY_START_ROW;
    uint32_t index = CURRENT_INDEX;
    enum count_type type;
    string str = "";
    string line = "";
    uint8_t note = 0;
    uint8_t count = 0;
    
    while (row <= max_row) {
        if (true != p_score->is_end(index)) {
            // grab the next note
            if ((0 != p_score->get_note_count(index++, note, type, count)) ||
                (!note_count_to_ascii(note, type, count, str))) {
                continue;
            }
            else if (SPACES_PER_NOTE > str.length()) {
                // pad with spaces to fully erase previous
                str += string(SPACES_PER_NOTE - str.length(), ' ');
            }
        }
        else {
            // pad with spaces to fully erase previous
            str = string(SPACES_PER_NOTE, ' ');
        }
        
        line += str;
        
        if (max_line_len <= line.length()) {
            // print the current row
            this->ui_.print(row, 0, A_NORMAL, line);
            line = "";
            row += 1;
        }
    }
}
    
void ApplicationManager::start_play_current_score(
    void)
{
    this->is_play_ = true;
    this->play_thread_ = thread(&ApplicationManager::play_main, this);
}
    
void ApplicationManager::stop_play_current_score(
    void)
{
    if (this->is_play_) {
        this->is_play_ = false;
        this->play_thread_.join();
        this->display_refresh();
    }
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

void ApplicationManager::enter_command(
    application_command command,
    string entry="N/A")
{
    MidiScore* const p_score = CURRENT_SCORE;
    enum count_type type;
    uint8_t note;
    uint8_t count;
    uint16_t bpm;
    
    switch (command) {
    case (CMD_BPM):
        if (is_number(entry)) {
            bpm = stoi(entry);
            p_score->set_bpm(bpm);
            }
            break;
    
    case (CMD_BPM_DECREMENT):
        p_score->get_bpm(bpm);
        if (bpm > 1) bpm--;
        p_score->set_bpm(bpm);
    
    case (CMD_BPM_INCREMENT):
        p_score->get_bpm(bpm);
        if (bpm < UINT16_MAX) bpm++;
        p_score->set_bpm(bpm);
        break;
    
    case (CMD_INDEX):
        if (is_number(entry)) {
            this->mutex_.lock();
            this->index_ = stoi(entry);
            this->mutex_.unlock();
        }
        break;
    
    case (CMD_INDEX_DECREMENT):
        this->mutex_.lock();
        if (0 != this->index_) this->index_--;
        this->mutex_.unlock();
        break;
    
    case (CMD_INDEX_INCREMENT):
        this->mutex_.lock();
        this->index_++;
        this->mutex_.unlock();
        break;
    
    case (CMD_MOVE):
        if (is_number(entry)) {
        //    app.display = stoi(entry);
        //    app.index = app.display;
        //    app.is_ui_refresh = true;
        }
        break;
    
    case (CMD_REPEAT):
        if (is_number(entry)) {
            p_score->set_repeat(stoi(entry));
            this->display_refresh_frame();
        }
        break;
    
    case (CMD_NOTE):
        if (ascii_to_note_count(entry, note, type, count)) {
            this->mutex_.lock();
            p_score->set_note_count(this->index_++, note, type, count);
            this->mutex_.unlock();
            this->display_refresh_current_score();
        }
        break;
    
    case (CMD_SAVE):
        p_score->save(entry);
        break;
    
    case (CMD_LOAD):
        p_score->load(entry);
        this->display_refresh();
        break;
    
    case (CMD_DELETE):
    case (CMD_PLAY):
    case (CMD_QUIT):
    case (CMD_INVALID):
        // do nothing
        break;
    }
}


