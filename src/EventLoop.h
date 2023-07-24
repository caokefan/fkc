#pragma once


#include "Channel.h"
#include "CurrentThread.h"
#include "NonCopyable.h"
#include "Poller.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

using namespace std;

using Task = function<void()>;
using ChannelList = vector<Channel*>;

class EventLoop : NonCopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void run_in_loop(Task task);
    void queue_in_loop(Task task);
    void wakeup();

    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);
    bool has_channel(Channel* channel);
    bool is_in_loop_thread() const { return threadId_ == Current_thread::tid(); }

private:
    void handle_read();
    void do_pending_tasks();

    atomic_bool looping_;
    atomic_bool quit_;
    const pid_t threadId_;

    unique_ptr<Poller> poller_;
    int wakeup_fd;
    unique_ptr<Channel> wakeup_channel_;

    ChannelList active_channels;
    atomic_bool has_pending_tasks_;
    vector<Task> pending_tasks_;

    mutex task_mutex_;
};