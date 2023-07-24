#include "Channel.h"
#include "EpollPoller.h"
#include <cstring>
#include <unistd.h>

const int k_new = -1;
const int k_added = 1;
const int k_deleted = 2;

EpollPoller::EpollPoller(EventLoop *loop) : Poller(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(k_init_event_list_size)
{
    if(epollfd_ < 0)
    {

    }
}

EpollPoller::~EpollPoller()
{
    close(epollfd_);
}

void EpollPoller::update_channel(Channel* channel)
{
    const int index = channel->index();

    if(index == k_new || index == k_deleted)
    {
        int sockfd = channel->get_fd();
        if(index == k_new)
        {
            channels_[sockfd] = channel;
        }

        channel->set_index(k_added);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int sockfd = channel->get_fd();
        if(channel->is_none_event())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(k_deleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::remove_channel(Channel *channel)
{
    int sockfd = channel->get_fd();
    int index = channel->index();

    channels_.erase(sockfd);

    if(index == k_added)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(k_deleted);
}

void EpollPoller::poll(int timeout, ChannelList *active_channels)
{
    int events_num = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeout);
    // int save_errno = errno;

    if(events_num > 0)
    {
        fill_active_channels(events_num, active_channels);
        if(events_num == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    } else if (events_num == 0) {

    } else {

    }
}

void EpollPoller::fill_active_channels(int events_num, ChannelList *active_channels) const 
{
    for(int i = 0; i < events_num; i++) 
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_real_events(events_[i].events);
        active_channels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel *channel) 
{
    int sockfd = channel->get_fd();

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->get_events();
    event.data.fd = sockfd;
    event.data.ptr = channel;

    if(epoll_ctl(epollfd_, operation, sockfd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {

        } else {
            
        }
    }

}