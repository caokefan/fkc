#include "EventLoop.h"
#include <iostream>
#include <sys/eventfd.h>

__thread EventLoop *t_loop_in_this_thread = nullptr;
const int k_poll_timeout = 10000;

int CreateEventFd() {
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(event_fd<0) cout << "error";
    return event_fd;
}

EventLoop::EventLoop() : looping_(false), quit_(false), has_pending_tasks_(false), 
    threadId_(Current_thread::tid()), poller_(Poller::new_defaultPoller(this)), 
    wakeup_fd(CreateEventFd()), wakeup_channel_(new Channel(this, wakeup_fd)) {
        if(t_loop_in_this_thread) {
            cout << "Another EventLoop" << this << "exists in this thread" << threadId_ << endl;
        } else {
            t_loop_in_this_thread = this;
        }

        wakeup_channel_->set_read_callback(bind(&EventLoop::handle_read, this));
        wakeup_channel_->enable_reading();
}

EventLoop::~EventLoop()
{
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    close(wakeup_fd);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    while(!quit_) {
        active_channels.clear();
        poller_->poll(k_poll_timeout, &active_channels);

        for(Channel *channel : active_channels) {
            channel->handle_event();
        }
        do_pending_tasks();
    }
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!is_in_loop_thread() || has_pending_tasks_)
    {
        wakeup();
    }
}

void EventLoop::run_in_loop(Task task)
{
    if(is_in_loop_thread())
    {
        task();
    } else {
        queue_in_loop(task);
    }
}

void EventLoop::queue_in_loop(Task task)
{
    {
        unique_lock<mutex> lock(task_mutex_);
        pending_tasks_.emplace_back(task);
    }

    if(!is_in_loop_thread() || has_pending_tasks_) 
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        cout << "EventLoop::wakeup() writes " << n << " bytes instead od 8\n";
    }
}

void EventLoop::update_channel(Channel *channel) {
    poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel *channel) {
    poller_->remove_channel(channel);
}

bool EventLoop::has_channel(Channel *channel)
{
    return poller_->has_channel(channel);
}

void EventLoop::handle_read()
{
    uint64_t one = 1;
    ssize_t n = read(wakeup_fd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        cout << "EventLoop::handle_read()\n";
    }
}

void EventLoop::do_pending_tasks()
{
    vector<Task> tasks;
    has_pending_tasks_ = true;
    {
        lock_guard<mutex> lock(task_mutex_);
        tasks.swap(pending_tasks_);
    }
    for(const Task &task: tasks) 
    {
        task();
    }
    has_pending_tasks_ = false;
}