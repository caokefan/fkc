#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const string &name)
    : baseloop_(baseloop), name_(name), started_(false), thread_nums_(0), next_(0)
    {}

EventLoopThreadPool::~EventLoopThreadPool(){}

void EventLoopThreadPool::start(const ThreadInitCallback &callback)
{
    started_ = true;
    if(thread_nums_==0)
    {
        callback(baseloop_);
    } else {
        for(int i=0;i<thread_nums_;++i)
        {
            char buffer[32] = {0};
            snprintf(buffer, sizeof(buffer), "%s %d", name_.c_str(), i);
            EventLoopThread *t = new EventLoopThread(callback, buffer);
            threads_.push_back(unique_ptr<EventLoopThread>(t));
            loops_.push_back(t->start_loop());
        }
    }
}

EventLoop* EventLoopThreadPool::get_next_eventloop()
{
    EventLoop* loop = baseloop_;
    if(!loops_.empty())
    {
        loop = loops_[next_++];
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

vector<EventLoop *> EventLoopThreadPool::get_all_loops()
{
    if (loops_.empty())
    {
        return vector<EventLoop *>(1, baseloop_);
    } else {
        return loops_;
    }
}