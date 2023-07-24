
#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

const int Channel::k_none_event_ = 0;
const int Channel::k_read_event_ = EPOLLIN | EPOLLPRI;
const int Channel::k_write_event_ = EPOLLOUT;

Channel::Channel(EventLoop * loop, int fd) : loop_(loop), fd_(fd), events_(0), real_events_(0), index_(-1), tied_(false) {}

void Channel::handle_event() {
    if(tied_) {
        shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handle_event_with_guard();
        }
    } else {
        handle_event_with_guard();
    }
}

void Channel::tie(const shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::remove() {
    loop_->update_channel(this);
}

void Channel::update() {
    loop_->update_channel(this);
}

void Channel::handle_event_with_guard() {
    if((real_events_ & EPOLLHUP) && !(real_events_ & EPOLLIN)) {
        if(close_callback_) close_callback_();
    }
    if(real_events_ & EPOLLERR) {
        if(error_callback_) error_callback_();
    }
    if(real_events_ & (EPOLLIN | EPOLLPRI)) {
        if(read_callback_) read_callback_();
    }
    if(real_events_ & EPOLLOUT) {
        if(write_callback_) write_callback_();
    }
}