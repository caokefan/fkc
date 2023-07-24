#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "TcpConnection.h"
#include <functional>
#include <sys/socket.h>
#include <sys/types.h>

using namespace placeholders;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if(loop==nullptr)
    {
        printf("TcpConnection: loop is nullptr");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const string name, int sockfd, const InetAddress &localaddr, const InetAddress &peeraddr)
    : loop_(CheckLoopNotNull(loop)), name_(name), state_(k_connecting), reading_(true), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)), localaddr_(localaddr), peeraddr_(peeraddr), highwater_mark_(64 * 1024 * 1024)
{
    channel_->set_read_callback(bind(&TcpConnection::handle_read, this));
    channel_->set_write_callback(bind(&TcpConnection::handle_write, this));
    channel_->set_close_callback(bind(&TcpConnection::handle_close, this));
    channel_->set_error_callback(bind(&TcpConnection::handle_error, this));

    socket_->set_keep_alive(true);
}

TcpConnection::~TcpConnection()
{
    printf("tcp connection::dtor[%s] at fd = %d state = %d \n", name_.c_str(), channel_->get_fd(), (int)state_);
}


void TcpConnection::send(const string &buf)
{
    if (state_ == k_connected)
    {
        if (loop_->is_in_loop_thread())
        {
            send_in_loop(buf);
        } else {
            loop_->run_in_loop(bind(&TcpConnection::send_in_loop, this, buf));
        }
    }
}

void TcpConnection::send_in_loop(const string &buf)
{
    ssize_t nwrote = 0;
    size_t remaining = buf.size(); 
    bool fault_error = false;

    if (state_ == k_disconnected)
    {
        printf("TcpConnection: disconnected,give up writing!\n");
        return;
    }

    if (!channel_->is_writting() && output_buffer_.readable_bytes() == 0)
    {
        nwrote = write(channel_->get_fd(), buf.c_str(), buf.size());
        if (nwrote >= 0)
        {
            remaining = buf.size() - nwrote;

            if (remaining == 0 && write_complete_callback_)
            {
                loop_->queue_in_loop(bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                printf("tcp connection::send in loop!\n");
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    fault_error = true;
                }
            }
        }
    }
    
    if (!fault_error && remaining > 0)
    {
        size_t oldlen = output_buffer_.readable_bytes();
        if (oldlen + remaining >= highwater_mark_ && oldlen < highwater_mark_)
        {
            loop_->queue_in_loop(bind(highwater_callback_, shared_from_this(), oldlen + remaining));
        }
        output_buffer_.append(buf.c_str() + nwrote, remaining);
        if (!channel_->is_writting())
        {
            channel_->enable_writting(); 
        }
    }
}


void TcpConnection::shutdown()
{
    if (state_ == k_connected)
    {
        set_state(k_disconnecting);
        loop_->run_in_loop(bind(&TcpConnection::shutdown_inLoop, this));
    }
}

void TcpConnection::shutdown_inLoop()
{
    if (!channel_->is_writting())
    {
        socket_->shutdown_write(); 
    }
}

void TcpConnection::establish_connect()
{
    set_state(k_connected);
    channel_->tie(shared_from_this());
    channel_->enable_reading(); 

    connection_callback_(shared_from_this());
}

void TcpConnection::destory_connect()
{
    if (state_ == k_connected)
    {
        set_state(k_disconnected);
        channel_->disable_all(); 
    }
    channel_->remove(); 
}

void TcpConnection::handle_read()
{
    int save_errno = 0;
    ssize_t n = input_buffer_.readfd(channel_->get_fd(), &save_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_);
    } else if (n == 0) {
        handle_close();
    } else {
        errno = save_errno;
        printf("tcp connection::handle read\n");
        handle_error();
    }
}

void TcpConnection::handle_write()
{
    if (channel_->is_writting())
    {
        int save_errno = 0;
        ssize_t n = output_buffer_.writefd(channel_->get_fd(), &save_errno);
        if (n > 0)
        {
            output_buffer_.retrieve(n);
            if (output_buffer_.readable_bytes() == 0)
            {
                channel_->disable_writting();
                if (write_complete_callback_)
                {
                    loop_->queue_in_loop(bind(write_complete_callback_, shared_from_this()));
                }

                if (state_ == k_disconnecting)
                {
                    shutdown_inLoop();
                }
            }
        } else {
            printf("tcp connection::handle write\n");
        }
    } else {
        printf("tcp connection fd=%d is down,no more send\n", channel_->get_fd());
    }
}

void TcpConnection::handle_close()
{
    set_state(k_disconnected);
    channel_->disable_all();

    TcpConnectionPtr connection_ptr(shared_from_this());

    connection_callback_(connection_ptr);
    close_callback_(connection_ptr);
}

void TcpConnection::handle_error()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;

    if (getsockopt(channel_->get_fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    } else {
        err = optval;
    }
    printf("tcp connection handle error name:%s  SO_ERROR:%d\n", name_.c_str(), err);
}
