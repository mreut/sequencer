/***** Inlcudes *****/

#include <sys/select.h>
#include <unistd.h>
#include <ncurses.h>
#include <cassert>
#include <cstring>

#include "UserInterface.hpp"


/***** Structs *****/

struct print_message_info {
    uint32_t x;
    uint32_t y;
    int attr;
    uint32_t len;
}; 


/***** Local Functions *****/

int32_t _poll_print(
    int read_fd,
    uint32_t& y,
    uint32_t& x,
    int& attr,
    uint8_t& text)
{
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 10000};
    struct print_message_info info {.x = 0, .y = 0, .len = 0};
    uint32_t len = sizeof(struct print_message_info);
    fd_set read_set;
    int32_t num_read = 0;
    
    FD_ZERO(&read_set);
    FD_SET(read_fd, &read_set);
    select(FD_SETSIZE, &(read_set), NULL, NULL, &timeout);
    if (FD_ISSET(read_fd, &read_set)) {
        num_read = read(read_fd, (void*) &info, len);
        if (len != num_read) {
            return -1;
        }
        
        num_read = read(read_fd, (void*) text, info.len);
        if (info.len != num_read) {
            return -1;
        }
        
        x = info.x;
        y = info.y;
        attr = info.attr;
    }
    
    return num_read;
}


/***** Class Methods *****/

UserInterface::UserInterface(
    void)
{
    int fd[2] = {0, 0};
    int r = 0;
    
    if (NULL == stdscr) {
        this->reference_count_ = 0;
        this->read_pipe_ = 0;
        this->write_pipe_ = 0;
        
        r = pipe2(fd, O_NONBLOCK);
        assert(r == 0);
        
        this->read_pipe_ = fd[0];
        this->write_pipe_ = fd[1];
    
        this->thread = thread(&UserInterface::display_main, this);
        

    }
    
    this->reference_count_++;
}
        
UserInterface::~UserInterface(
    void)
{
    this->reference_count_--;
    
    if (0 >= this->reference_count_) {
        
        this->thread_.join();
        
        close(this->read_pipe_); 
        close(this->write_pipe_); 
    }
}

int32_t UserInterface::print(
    uint32_t y,
    uint32_t x,
    int attr,
    string text)
{
    struct print_message_info info;
    int32_t len = sizeof(struct print_message_info);
    int32_t r = -1;
    
    if (0 == text.length()) {
        return -1;
    }
    
    info.y = y;
    info.x = x;
    info.attr = attr;
    
    this->mutex_.lock();
    
    if (len == write(this->write_pipe_, &info, len) {
        // length of string plus one for null terminating character
        len = text.length() + 1;
        if (len == write(this->write_pipe_, text.c_str(), len) {
            r = 0;
        }
    }
    
    this->mutex_.unlock();
    
    return -1;
}

void UserInterface::display_main(
    void)
{
    static uint8_t text[USER_INTERFACE_PRINT_MAX_LEN];
    uint32_t x = 0;
    uint32_t y = 0;
    int attr = A_NORMAL;
    int32_t n = 0;
    
    initscr();
    raw();                  /* Line buffering disabled */
    keypad(stdscr, TRUE);   /* We get F1, F2 etc.. */
    noecho();

    refresh();
    curs_set(0);

    while (0 >= this->reference_count_) {
        
        n = _poll_print(this->read_pipe_, y, x, attr, text);
        if (n > 0) {
            attron(attr);
            mvprintw(y, x, "%s", text);
            attroff(attr);
        }
    }

    endwin();
}
