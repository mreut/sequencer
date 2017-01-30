#ifndef __SOCKET_HPP
#define __SOCKET_HPP

/***** Includes *****/

#include <string>
#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


/***** Namespace *****/

using namespace std;


/***** Classes *****/

class MulticastSocket {
    
    public:
        MulticastSocket(
            void);
        
        ~MulticastSocket(
            void);
            
        bool master_open(
            string ip_address,
            uint16_t port);
        
        bool master_send(
            uint8_t* buf,
            uint32_t buf_len);
        
        bool slave_open(
            string ip_address,
            uint16_t port);
    
        bool slave_recv(
            uint8_t* buf,
            uint32_t buf_len,
            struct timeval timeout);
    
        bool is_master(
            void);
    
        bool is_slave(
            void);
    
    private:
        struct sockaddr_in addr_;
        struct ip_mreq group_;
        int fd_;
        string ip_address_;
        uint16_t port_;
        bool is_master_;
        bool is_slave_;
};

#endif
