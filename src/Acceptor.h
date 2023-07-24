#pragma once

#include "NonCopyable.h"
#include "Channel.h"
#include "Socket.h"
#include <functional>

class EventLoop;
class InetAddress;

using namespace std;
using NewConnectionCallback = function<void(int sockfd, const InetAddress &address)>;

class Acceptor : NonCopyable
{
public:
    Acceptor(EventLoop *loop, const InetAddress &listenaddr, bool reuseport);
    ~Acceptor();
    void set_new_connection_callback(const NewConnectionCallback &cb)
    {
        new_connection_callback_ = cb;
    }
    bool is_listening() { return listening_; }
    void listen();

private:
    void handle_read();

    EventLoop *loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listening_;
};