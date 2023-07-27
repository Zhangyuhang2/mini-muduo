

#include <sys/timerfd.h>
#include <inttypes.h>
#include <stdio.h>
#include <strings.h>

#include "TimerQueue.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Timer.h"


#include <iostream>
#include <string>

using namespace std;


TimerQueue::TimerQueue(EventLoop* pLoop)
    :_timerfd(createTimerfd())
    ,_pLoop(pLoop)
    ,_pTimerfdChannel(new Channel(pLoop, _timerfd)) // Memory Leak !!!
{
    _pTimerfdChannel->setReadCallback(std::bind(&TimerQueue::handleRead,this));
    _pTimerfdChannel->enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(_timerfd);
}


void TimerQueue::doAddTimer(Timer* pTimer)
{
    bool earliestChanged = insert(pTimer);
    if(earliestChanged)
    {
        resetTimerfd(_timerfd, pTimer->getStamp());
    }
}



Timer* TimerQueue::addTimer(TimerCallback pRun, Timestamp when, double interval)
{
    Timer* pAddTimer = new Timer(when, pRun, interval); //Memory Leak !!!
    _pLoop->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, pAddTimer));
    return pAddTimer;
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged = insert(timer);
    if (earliestChanged)
    {
    resetTimerfd(_timerfd, timer->getStamp());
    }
}


void TimerQueue::cancelTimer(Timer* pTimer)
{
    Entry e(pTimer->getId(), pTimer);
    TimerList::iterator it;
    for(it = _pTimers.begin(); it != _pTimers.end(); ++it)
    {
        if(it->second == pTimer)
        {
            _pTimers.erase(it);
            break;
        }
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(_timerfd, now);

    vector<Entry> expired = getExpired(now);
    vector<Entry>::iterator it;
    for(it = expired.begin(); it != expired.end(); ++it)
    {
        it->second->timeout();
    }
    reset(expired, now);
}

void TimerQueue::handleWrite()
{}

int TimerQueue::createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
            TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        cout << "failed in timerfd_create" << endl;
    }
    return timerfd;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(0xffffffff));
    TimerList::iterator end = _pTimers.lower_bound(sentry);
    copy(_pTimers.begin(), end, back_inserter(expired));
    _pTimers.erase(_pTimers.begin(), end);
    return expired;
}

void TimerQueue::readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    if (n != sizeof(howmany))
    {
        cout << "Timer::readTimerfd() error " << endl;
    }
}

void TimerQueue::reset(const vector<Entry>& expired, Timestamp now)
{
    vector<Entry>::const_iterator it;
    for(it = expired.begin(); it != expired.end(); ++it)
    {
        if(it->second->isRepeat())
        {
            it->second->moveToNext();
            insert(it->second);
        }
    }

    Timestamp nextExpire;
    if(!_pTimers.empty())
    {
        nextExpire = _pTimers.begin()->second->getStamp();
    }
    if(nextExpire.valid())
    {
        resetTimerfd(_timerfd, nextExpire);
    }
}

void TimerQueue::resetTimerfd(int timerfd, Timestamp stamp)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(stamp);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret)
    {
        cout << "timerfd_settime error" << endl;
    }
}

bool TimerQueue::insert(Timer* pTimer)
{
    bool earliestChanged = false;
    Timestamp when = pTimer->getStamp();
    TimerList::iterator it = _pTimers.begin();
    if(it == _pTimers.end() || when < it->first)
    {
        earliestChanged = true;
    }
    pair<TimerList::iterator, bool> result
       = _pTimers.insert(Entry(when, pTimer));
    if(!(result.second))
    {
        cout << "_pTimers.insert() error " << endl;
    }

    return earliestChanged;
}

struct timespec TimerQueue::howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
        - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
