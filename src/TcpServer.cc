#include "TcpServer.h"
#include <string.h>
#include <stdio.h>

using namespace placeholders;

#define BUFFER_SIZE64 64

EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        printf("TcpServer: loop is nullptr");
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenaddr, const string &name, Option option)
    : loop_(CheckLoopNotNull(loop)), ip_port_(listenaddr.get_ip_port()), name_(name), acceptor_(new Acceptor(loop, listenaddr, option == k_reuse_port)), thread_pool_(new EventLoopThreadPool(loop, name_)), connection_callback_(), message_callback_(), next_conn_id_(1), started_(0)
{
    acceptor_->set_new_connection_callback(bind(&TcpServer::new_connection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    for (auto &it : connections_)
    {
        TcpConnectionPtr conn(it.second); 
        it.second.reset();

        conn->get_loop()->run_in_loop(bind(&TcpConnection::destory_connect, conn));
    }
}


void TcpServer::set_thread_num(int thread_num)
{
    thread_pool_->set_thread_num(thread_num);
}


void TcpServer::start()
{
    if (started_++ == 0) 
    {
        thread_pool_->start(thread_init_callback_);
        loop_->run_in_loop(bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::new_connection(int sockfd, const InetAddress &peeraddr)
{
    EventLoop *ioloop = thread_pool_->get_next_eventloop();

    char buffer[BUFFER_SIZE64] = {0};
    snprintf(buffer, sizeof(buffer), "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    string conn_name = name_ + buffer;

    sockaddr_in local;
    memset(&local, 0, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        printf("new connection get localaddr error");
    }

    InetAddress localaddr(local);
    TcpConnectionPtr conn(new TcpConnection(ioloop, conn_name, sockfd, localaddr, peeraddr));

    connections_[conn_name] = conn;

    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);

    conn->set_close_callback(bind(&TcpServer::remove_connection, this, _1));
    ioloop->run_in_loop(bind(&TcpConnection::establish_connect, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr &conn)
{
    loop_->run_in_loop(bind(&TcpServer::remove_connection_inLoop, this, conn));
}

void TcpServer::remove_connection_inLoop(const TcpConnectionPtr &conn)
{
    connections_.erase(conn->get_name());
    EventLoop *ioloop = conn->get_loop();
    ioloop->queue_in_loop(bind(&TcpConnection::destory_connect, conn));
}