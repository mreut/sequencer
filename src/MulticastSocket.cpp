/***** Includes *****/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>


/***** Class Methods *****/

MulticastSocket::MulticastSocket(
    void)
{
    this->fd_;
    this->ip_address_ = "";
    uint16_t port_ = 0;
    bool is_master_ = false;
    bool is_slave_ = false;
}

MulticastSocket::~MulticastSocket(
    void)
{
    
}

bool MulticastSocket::open_master(
    string ip_address,
    uint16_t port,
    uint16_t packet_length)
{
    if ((this->is_master_) || (this->is_slave_)) {
        return false;
    }
    
    this->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > this->fd) {
        return false;
    }
    
    memset((char*) this->addr_, 0, sizeof(this->addr_));
    this->addr_.sin_family = AF_INET;
    this->addr_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    this->addr_.sin_port = htons(port);
    
    // disable loopback
    
    
}
    
bool MulticastSocket::open_slave(
    string ip_address,
    uint16_t port,
    uint16_t packet_length);
