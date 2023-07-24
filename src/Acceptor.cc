#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <sys/types.h>

static int create_nonblock_sockfd()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
    {
        printf("Acceptor: sockfd < 0\n");
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress &listenaddr, bool reuseport)
    : loop_(loop), accept_socket_(create_nonblock_sockfd()), accept_channel_(loop, accept_socket_.get_fd()), listening_(false)
{
    accept_socket_.set_reuse_addr(true);
    accept_socket_.set_reuse_port(true);
    accept_socket_.bind_address(listenaddr);
    accept_channel_.set_read_callback(bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor()
{
    accept_channel_.disable_all();
    accept_channel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
}

void Acceptor::handle_read()
{
    InetAddress peeraddr;
    int connfd = accept_socket_.accept(&peeraddr);
    if(connfd > 0)
    {
        if(new_connection_callback_)
        {
            new_connection_callback_(connfd, peeraddr);
        } else {
            close(connfd);
        }
    } else {
        printf("connfd <= 0\n");
    }
}