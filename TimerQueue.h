
#pragma once
#include "Timestamp.h"
#include "Timer.h"
#include "EventLoop.h"

#include "Channel.h"
#include "EventLoop.h"

#include <vector>
#include <set>
using namespace std;

class TimerQueue 
{
    public:

        TimerQueue(EventLoop *pLoop);
        ~TimerQueue();
        void doAddTimer(Timer* timer);
        Timer* addTimer(TimerCallback cb,
                Timestamp when,
                double interval);
        void cancelTimer(Timer* pTimer);
        void handleRead();
        void handleWrite();

    private:
        void addTimerInLoop(Timer* timer);
        void cancelInLoop(long timerId);
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        int createTimerfd();
        vector<TimerQueue::Entry> getExpired(Timestamp now);
        void readTimerfd(int timerfd, Timestamp now);
        void reset(const vector<Entry>& expired, Timestamp now);
        void resetTimerfd(int timerfd, Timestamp stamp);
        bool insert(Timer* pItem);
        struct timespec howMuchTimeFromNow(Timestamp when);

        int _timerfd;
        TimerList _pTimers;
        Channel* _pTimerfdChannel;
        EventLoop* _pLoop;
        
};
