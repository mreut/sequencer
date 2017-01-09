#ifndef __USER_INTERFACE_HPP
#define __USER_INTERFACE_HPP

/***** Includes *****/

#include <cstdint>
#include <string>
#include <mutex>
#include <thread>


/***** Defines *****/

#define USER_INTERFACE_PRINT_MAX_LEN 256


/***** Namespace *****/

using namespace std;


/***** Classes *****/

class UserInterface {
    public:
        UserInterface(
            void);
        
        ~UserInterface(
            void);
        
        int32_t print(
            uint32_t y,
            uint32_t x,
            string text);
        
    private:
        void display_main(
            void);

        static int32_t reference_count_;
        static int read_pipe_;
        static int write_pipe_;
        static mutex mutex_;
        static thread thread_;
}
