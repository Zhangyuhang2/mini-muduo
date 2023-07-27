#include <mutex>
#include <condition_variable>
#include "noncopyable.h"
using namespace std;

class CountDownLatch : public noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable mutex mutex_;
  condition_variable cv;
  int count_;
};