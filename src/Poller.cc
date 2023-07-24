#include "Poller.h"

Poller::Poller(EventLoop *loop) : owner_loop_(loop) {}

bool Poller::has_channel(Channel * channel) const
{
    auto it = channels_.find(channel->get_fd());
    return it != channels_.end() && it->second == channel;
}