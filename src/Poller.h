#pragma once

#include "NonCopyable.h"
#include "Channel.h"
#include <unordered_map>
#include <vector>

using ChannelList = vector<Channel*>;
using ChannelMap = unordered_map<int, Channel*>;

class Poller : NonCopyable {
public:
    Poller(EventLoop* loop);
    virtual ~Poller() = default;
    virtual void poll(int timeout, ChannelList * active_channels) = 0;
    bool has_channel(Channel *Channel) const;

    static Poller *new_defaultPoller(EventLoop *loop);
    virtual void update_channel(Channel *channel) = 0;
    virtual void remove_channel(Channel *channel) = 0;

protected:
    ChannelMap channels_;

private:
    EventLoop *owner_loop_;

};