#include "Poller.h"
#include "EpollPoller.h"
#include <cstdlib>

Poller *Poller::new_defaultPoller(EventLoop *loop)
{
    if (getenv("MUDUO_USE_POLL"))
    {
        return nullptr; 
    }
    else
    {
        return new EpollPoller(loop); 
    }
}