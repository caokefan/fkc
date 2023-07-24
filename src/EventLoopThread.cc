#include "EventLoop.h"
#include "EventLoopThread.h"
#include <memory>

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const string &name)
    : loop_(nullptr), exiting_(false), thread_(bind(&EventLoopThread::thread_function, this), name), thread_mutex_(), condition_(), callback_function_(cb)
    {}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::start_loop()
{
    thread_.start();
    EventLoop *loop = nullptr;
    {
        unique_lock<mutex> lock(thread_mutex_);
        while(loop_==nullptr)
        {
            condition_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::thread_function()
{
    EventLoop loop;
    if(callback_function_)
    {
        callback_function_(&loop);
    }
    {
        unique_lock<mutex> lock(thread_mutex_);
        loop_ = &loop;
        condition_.notify_one();
    }

    loop.loop();

    unique_lock<mutex> lock(thread_mutex_);
    loop_ = nullptr;
}