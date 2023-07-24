#include "Poller.h"
#include <sys/epoll.h>
#include <vector>

using EventList = vector<epoll_event>;

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    void poll(int timeout, ChannelList *active_channels) override;

    void update_channel(Channel *channel);
    void remove_channel(Channel *channel);
private:
    void fill_active_channels(int events_num, ChannelList *active_channels) const;
    void update(int operation, Channel *channel);

    static const int k_init_event_list_size = 16;
    int epollfd_;
    EventList events_;
};