#include "Socket.h"
#include "InetAddress.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Socket::~Socket()
{
    close(sockfd_);
}

void Socket::bind_address(const InetAddress &localaddr)
{
    if( bind(sockfd_, (sockaddr *)localaddr.get_sockaddr(), sizeof(sockaddr_in)) != 0)
    {

    }
}

void Socket::listen()
{
    if(::listen(sockfd_, 1024) != 0)
    {

    }
}

int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        peeraddr->set_sockaddr(addr);
    }
    return connfd;
}

void Socket::shutdown_write()
{
    if(::shutdown(sockfd_, SHUT_WR) < 0)
    {

    }
}

void Socket::set_tcp_no_delay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::set_reuse_addr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::set_reuse_port(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::set_keep_alive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}