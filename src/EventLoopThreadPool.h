#pragma once

#include "NonCopyable.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class EventLoop;
class EventLoopThread;

using namespace std;
using ThreadInitCallback = function<void(EventLoop*)>;


class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop *baseloop, const string &name);
    ~EventLoopThreadPool();

    void set_thread_num(int thread_nums) { thread_nums_ = thread_nums; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    EventLoop *get_next_eventloop();
    vector<EventLoop*> get_all_loops();
    bool is_started() const { return started_; }
    string get_name() const { return name_; }
private:
    EventLoop *baseloop_;
    string name_;
    bool started_;
    int thread_nums_;
    int next_;
    vector<unique_ptr<EventLoopThread>> threads_;
    vector<EventLoop*> loops_;
};