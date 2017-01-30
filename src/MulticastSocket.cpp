/***** Includes *****/

#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include "MulticastSocket.hpp"


/***** Class Methods *****/

MulticastSocket::MulticastSocket(
    void)
{
    this->ip_address_ = "";
    this->port_ = 0;
    this->is_master_ = false;
    this->is_slave_ = false;
}

MulticastSocket::~MulticastSocket(
    void)
{
    if ((this->is_master_) || (this->is_slave_)) {
        close(this->fd_);
    }
}

bool MulticastSocket::master_open(
    string ip_address,
    uint16_t port)
{
    if ((this->is_master_) || (this->is_slave_)) {
        return false;
    }
    
    this->fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > this->fd_) {
        return false;
    }
    
    memset((char*) &(this->addr_), 0, sizeof(this->addr_));
    this->addr_.sin_family = AF_INET;
    this->addr_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    this->addr_.sin_port = htons(port);
    this->ip_address_ = ip_address;
    this->port_ = port;
    
    this->is_master_ = true;
    
    return true;
}
    
bool MulticastSocket::slave_open(
    string ip_address,
    uint16_t port)
{
    u_int ttl = 1;
    int32_t r = 0;
    
    if ((this->is_master_) || (this->is_slave_)) {
        return false;
    }
    
    this->fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > this->fd_) {
        return false;
    }
    
    r = setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, &ttl, sizeof(ttl));
    if (0 != r) {
        return false;
    }
    
    memset((char*) &(this->addr_), 0, sizeof(this->addr_));
    this->addr_.sin_family = AF_INET;
    this->addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    this->addr_.sin_port = htons(port);
    
    r = bind(this->fd_,
             (struct sockaddr*) &(this->addr_),
             sizeof(this->addr_));
    if (0 != r) {
        return false;
    }
    
    this->group_.imr_multiaddr.s_addr = inet_addr(ip_address.c_str());
    this->group_.imr_interface.s_addr = htonl(INADDR_ANY);
    r = setsockopt(this->fd_,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &(this->group_),
                   sizeof(this->group_));
    if (0 != r) {
        return false;
    }
    
    this->ip_address_ = ip_address;
    this->port_ = port;
    this->is_slave_ = true;
    
    return true;
}


bool MulticastSocket::master_send(
    uint8_t* buf,
    uint32_t buf_len)
{
    uint32_t len = 0;
    bool success = false;
    
    if (this->is_master_) {
        len = sendto(this->fd_,
                     buf,
                     buf_len,
                     0,
                     (struct sockaddr *) &(this->addr_),
                     sizeof(this->addr_));
        
        if (len == buf_len) {
            success = true;
        }
    }
    
    return success;
}

bool MulticastSocket::slave_recv(
    uint8_t* buf,
    uint32_t buf_len,
    struct timeval timeout)
{
    fd_set read_set;
    int32_t num_read = 0;
    
    FD_ZERO(&read_set);
    FD_SET(this->fd_, &read_set);
    select(FD_SETSIZE, &(read_set), NULL, NULL, &timeout);
    if (FD_ISSET(this->fd_, &read_set)) {
        num_read = read(this->fd_, (void*) buf, buf_len);
    }
    
    if (((int32_t) buf_len) != num_read) {
        return false;
    }
    
    return true;
}

bool MulticastSocket::is_master(
    void)
{
    return this->is_master_;
}

bool MulticastSocket::is_slave(
    void)
{
    return this->is_slave_;
}
