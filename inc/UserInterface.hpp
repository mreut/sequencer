#ifndef __USER_INTERFACE_HPP
#define __USER_INTERFACE_HPP

/***** Includes *****/

#include <ncurses.h>
#include <cstdint>
#include <string>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>


/***** Defines *****/

#define USER_INTERFACE_PRINT_MAX_LEN 256


/***** Namespace *****/

using namespace std;


/***** Classes *****/

class UserInterface {
    public:
        ~UserInterface(
            void);
        
        void start(
            void);
        
        int32_t clear(
            void);
        
        int32_t print(
            uint32_t y,
            uint32_t x,
            int attr,
            string& text);
        
        int32_t get_input(
            int32_t& in);
        
        int32_t get_rows(
            void);
            
        int32_t get_cols(
            void);
        
    private:
        void ncurses_main(
            void);

        static int32_t reference_count_;
        static int32_t cols_;
        static int32_t rows_;
        static int read_pipe_;
        static int write_pipe_;
        static condition_variable condition_;
        static mutex mutex_;
        static thread thread_;
        static queue<int32_t> fifo_;
};

#endif
