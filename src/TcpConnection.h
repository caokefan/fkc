#pragma once

#include "Buffer.h"
#include "Callbacks.h"
#include "NonCopyable.h"
#include "InetAddress.h"
#include <atomic>
#include <memory>
#include <string>

class Channel;
class EventLoop;
class Socket;

using namespace std;

class TcpConnection : NonCopyable, public enable_shared_from_this<TcpConnection>
{
    enum StateE
    {
        k_disconnected,
        k_connecting,
        k_connected,
        k_disconnecting,
    };
public:
    TcpConnection(EventLoop *loop, const string name, int sockfd, const InetAddress &localaddr, const InetAddress &peeraddr);
    ~TcpConnection();

    EventLoop *get_loop() const { return loop_; }
    const string &get_name() const { return name_; }
    const InetAddress &get_localaddr() const { return localaddr_; }
    const InetAddress &get_peeraddr() const { return peeraddr_; }
    bool connected() { return state_ == k_connected; }
    void set_state(StateE state) { state_ = state; }
    void send(const string& buf);
    void shutdown();
    void establish_connect();
    void destory_connect();
    void set_close_callback(const CloseCallback &callback) { close_callback_ = callback; }
    void set_highwater_callback(const HighWaterMarkCallback &callback) { highwater_callback_ = callback; }
    void set_connection_callback(const ConnectionCallback &callback) { connection_callback_ = callback; }
    void set_message_callback(const MessageCallback &callback) { message_callback_ = callback; }
    void set_write_complete_callback(const WriteCompleteCallback &callback) { write_complete_callback_ = callback; }
private:
    void handle_read();
    void handle_write();
    void handle_close();
    void handle_error();
    void send_in_loop(const string& buf);
    void shutdown_inLoop();

    EventLoop *loop_; 
    const string name_;

    atomic_int state_;
    bool reading_;

    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;

    const InetAddress localaddr_;
    const InetAddress peeraddr_;

    ConnectionCallback connection_callback_;        
    MessageCallback message_callback_;              
    WriteCompleteCallback write_complete_callback_; 
    CloseCallback close_callback_;

    HighWaterMarkCallback highwater_callback_;
    size_t highwater_mark_; 

    Buffer input_buffer_;  
    Buffer output_buffer_;    
};

