#pragma once

#include "Acceptor.h"
#include "Buffer.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "NonCopyable.h"
#include "TcpConnection.h"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

using ThreadInitCallback = function<void(EventLoop *)>;
using ConnectionMap = unordered_map<string, TcpConnectionPtr>;

class TcpServer : NonCopyable
{
public:
    enum Option
    {
        k_noReuse_port, 
        k_reuse_port,   
    };

public:
    TcpServer(EventLoop *loop, const InetAddress &listenaddr, const string &name, Option option = k_noReuse_port);
    ~TcpServer();
    void set_thread_init_callback(const ThreadInitCallback &callback) { thread_init_callback_ = callback; }
    void set_connection_callback(const ConnectionCallback &callback) { connection_callback_ = callback; }
    void set_message_callback(const MessageCallback &callback) { message_callback_ = callback; }
    void set_write_complete_callback(const WriteCompleteCallback &callback) { write_complete_callback_ = callback; }
    void set_thread_num(int thread_num);
    void start();

private:
    void new_connection(int sockfd, const InetAddress &peeraddr);
    void remove_connection(const TcpConnectionPtr &conn);
    void remove_connection_inLoop(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    const string ip_port_;
    const string name_;
    unique_ptr<Acceptor> acceptor_;               
    shared_ptr<EventLoopThreadPool> thread_pool_; 
    ConnectionCallback connection_callback_;        
    MessageCallback message_callback_;              
    WriteCompleteCallback write_complete_callback_; 
    ThreadInitCallback thread_init_callback_; 
    atomic_int started_;
    int next_conn_id_;
    ConnectionMap connections_; 
};