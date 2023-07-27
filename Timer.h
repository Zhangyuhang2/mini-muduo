
#ifndef TIMER_H
#define TIMER_H

#include "Timestamp.h"
#include <iostream>
#include <functional>


typedef std::function<void()> TimerCallback;

class Timer
{
    public:
        Timer(Timestamp stamp, TimerCallback cb, double interval)
            :_stamp(stamp)
             ,_id(stamp)
             ,_cb(cb)
             ,_interval(interval)
    {}
        Timestamp getStamp()
        {
            return _stamp;
        }
        Timestamp getId()
        {
            return _id;
        }
        void timeout()
        {
            _cb();
        }

        bool isRepeat()
        {
            return _interval > 0.0;
        }

        void moveToNext()
        {
            _stamp = Timestamp::nowAfter(_interval);
        }
    private:
        Timestamp _stamp;
        Timestamp _id;
        TimerCallback _cb;
        double _interval;//seconds
};

#endif