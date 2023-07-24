#pragma once

#include "NonCopyable.h"
#include "Thread.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

class EventLoop;
using namespace std;

using ThreadInitCallback = function<void(EventLoop*)>;

class EventLoopThread : NonCopyable
{
public:
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const string &name = string());
    ~EventLoopThread();
    EventLoop *start_loop();
private:
    void thread_function();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    mutex thread_mutex_;
    condition_variable condition_;
    ThreadInitCallback callback_function_;
};