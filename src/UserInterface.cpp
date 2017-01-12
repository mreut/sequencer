/***** Inlcudes *****/

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ncurses.h>
#include <cassert>
#include <cstring>

#include "UserInterface.hpp"


/***** Defines *****/

#define POLL_PRINT_TIMEOUT_SEC 0
#define POLL_PRINT_TIMEOUT_USEC 10000


/***** Structs *****/

struct print_message_info {
    uint32_t x;
    uint32_t y;
    int attr;
    uint32_t len;
}; 


/***** Static Members *****/

int32_t UserInterface::reference_count_ = 0;
int UserInterface::read_pipe_ = 0;
int UserInterface::write_pipe_ = 0;
mutex UserInterface::mutex_;
thread UserInterface::thread_;
queue<int32_t> UserInterface::fifo_;


/***** Local Functions *****/

int32_t _poll_print(
    int read_fd,
    uint32_t& y,
    uint32_t& x,
    int& attr,
    uint8_t* p_text)
{
    struct timeval timeout;
    struct print_message_info info;
    int32_t len = sizeof(struct print_message_info);
    fd_set read_set;
    int32_t num_read = 0;
    
    timeout.tv_sec = POLL_PRINT_TIMEOUT_SEC;
    timeout.tv_usec = POLL_PRINT_TIMEOUT_USEC;
    
    info.x = 0;
    info.y = 0;
    info.len = 0;
    info.attr = A_NORMAL;
    
    FD_ZERO(&read_set);
    FD_SET(read_fd, &read_set);
    select(FD_SETSIZE, &(read_set), NULL, NULL, &timeout);
    if (FD_ISSET(read_fd, &read_set)) {
        num_read = read(read_fd, (void*) &info, len);
        if (len != num_read) {
            return -1;
        }
        
        len = info.len;
        num_read = read(read_fd, (void*) p_text, len);
        if (len != num_read) {
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
        this->reference_count_ = 1;
        this->read_pipe_ = 0;
        this->write_pipe_ = 0;
        
        r = pipe2(fd, O_NONBLOCK);
        assert(r == 0);
        
        this->read_pipe_ = fd[0];
        this->write_pipe_ = fd[1];
    
        this->thread_ = thread(&UserInterface::ncurses_main, this);
    }
    else {
        this->reference_count_++;
    }
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
    string& text)
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
    // length of string plus one for null terminating character
    info.len = text.length() + 1;
    
    this->mutex_.lock();
    
    if (len == write(this->write_pipe_, &info, len)) {
        len = info.len;
        if (len == write(this->write_pipe_, text.c_str(), len)) {
            r = 0;
        }
    }
    
    this->mutex_.unlock();
    
    return r;
}

int32_t UserInterface::get_input(
    int32_t& in)
{
    int32_t r = -1;
    
    this->mutex_.lock();
    if (0 != this->fifo_.size()) {
        in = this->fifo_.front();
        this->fifo_.pop();
        r = 0;
    }
    this->mutex_.unlock();
    
    return r;
}

void UserInterface::ncurses_main(
    void)
{
    static uint8_t text[USER_INTERFACE_PRINT_MAX_LEN];
    uint32_t x = 0;
    uint32_t y = 0;
    int attr = A_NORMAL;
    int32_t n = 0;
    
    initscr();
    raw();                  /* Line buffering disabled */
    noecho();
    keypad(stdscr, TRUE);   /* We get F1, F2 etc.. */
    nodelay(stdscr, TRUE);

    refresh();
    curs_set(0);

    while (0 < this->reference_count_) {
        
        n = getch();
        if (ERR != n) {
            // capitalize all alphabetical input
            if (isalpha(n)) n &= ~0x20;
            this->mutex_.lock();
            this->fifo_.push(n);
            this->mutex_.unlock();
        }
        
        n = _poll_print(this->read_pipe_, y, x, attr, text);
        if (n > 0) {
            attron(attr);
            mvprintw(y, x, "%s", text);
            attroff(attr);
        }
    }

    endwin();
}
