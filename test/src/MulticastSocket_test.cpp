#include <iostream>
#include <unistd.h>
#include "MulticastSocket.hpp"

int main(
    int argc,
    char* argv[])
{
    MulticastSocket socket;
    string str = "Hello World!";
    char buf[256];
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (!argv[1]) {
        cout << argv[0] << "<0=master | 1=slave>\n";
        return -1;
    }
    
    if ('0' == argv[1][0]) {
        
        if (!socket.master_open("225.0.0.37", 12345)) {
            cout << "failed to open master socket!\n";
            return -1;
        }
        while (1) {
            socket.master_send((uint8_t*) str.c_str(), str.length() + 1);
            sleep(1);
        }
    }
    else if ('1' == argv[1][0]) {
        if (!socket.slave_open("225.0.0.37", 12345)) {
            cout << "failed to open slave socket!\n";
            return -1;
        }
        while (1) {
            if (socket.slave_recv((uint8_t*) buf, str.length() + 1, timeout)) {
                buf[255] = '\0';
                string tmp(buf);
                cout << tmp << "\n";
            }
        }
    }
    
    return 0;
}

