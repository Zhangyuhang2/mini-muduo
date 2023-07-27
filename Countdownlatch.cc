// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Countdownlatch.h"

using namespace std;

CountDownLatch::CountDownLatch(int count)
  :count_(count)
{
}

void CountDownLatch::wait()
{
  std::unique_lock<std::mutex> lock(mutex_);
  while (count_ > 0)
  {
    cv.wait(lock);
  }
}

void CountDownLatch::countDown()
{
  std::unique_lock<std::mutex> lock(mutex_);
  --count_;
  if (count_ == 0)
  {
    cv.notify_all();
  }
}

int CountDownLatch::getCount() const
{
  std::unique_lock<std::mutex> lock(mutex_);
  return count_;
}

