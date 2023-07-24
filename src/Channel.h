#pragma once

#include <functional>
#include <memory>

class EventLoop;
using namespace std;
using EventCallback = function<void()>;

class Channel {
public:
    Channel(EventLoop* loop, int fd);

    void handle_event();
    void tie(const shared_ptr<void> &);
    int get_fd() const { return fd_;}
    int get_events() const { return events_; }
    int set_real_events(int event) { real_events_ = events_;}
    EventLoop* owner_loop() { return loop_; }
    void remove();
    int index() { return index_; }
    void set_index(int index) { index_ = index; }
    void enable_reading() {
        events_ |= k_read_event_;
        update();
    }
    void disable_reading() {
        events_ &= ~k_read_event_;
        update();
    }
    void enable_writting() {
        events_ |= k_write_event_;
        update();
    }
    void disable_writting() {
        events_ &= ~k_write_event_;
        update();
    }
    void disable_all() {
        events_ = k_none_event_;
        update();
    }

    bool is_none_event() const { return events_ == k_none_event_; }
    bool is_writting() const { return events_ & k_write_event_; }
    bool is_reading() const { return events_ & k_read_event_; }

    void set_read_callback(EventCallback callback) { read_callback_ = move(callback); }
    void set_write_callback(EventCallback callback) { write_callback_ = move(callback); }
    void set_close_callback(EventCallback callback) { close_callback_ = move(callback); }
    void set_error_callback(EventCallback callback) { error_callback_ = move(callback); }

private:
    void update();
    void handle_event_with_guard();

    static const int k_none_event_;
    static const int k_read_event_;
    static const int k_write_event_;

    EventLoop *loop_;
    const int fd_;
    int events_;
    int real_events_;
    int index_;

    weak_ptr<void> tie_;
    bool tied_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};