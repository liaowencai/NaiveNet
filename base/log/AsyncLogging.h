#ifndef NAIVENET_BASE_ASYNCLOGGING_H
#define NAIVENET_BASE_ASYNCLOGGING_H

#include "../BlockingQueue.h"
#include <muduo/base/BoundedBlockingQueue.h>
#include "../CountDownLatch.h"
#include "../Mutex.h"
#include "../Thread.h"

#include "./LogStream.h"

#include <functional>
#include <string>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>

namespace NaiveNet
{

class AsyncLogging
{
 /*public:
	AsyncLogging(const AsyncLogging&) = delete;
	AsyncLogging& operator=(const AsyncLogging&) = delete;*/

 public:

  AsyncLogging(const std::string& basename,
               size_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  // declare but not define, prevent compiler-synthesized functions
  AsyncLogging(const AsyncLogging&);  // ptr_container
  void operator=(const AsyncLogging&);  // ptr_container

  void threadFunc();

  typedef NaiveNet::detail::FixedBuffer<NaiveNet::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  std::string basename_;
  size_t rollSize_;
  NaiveNet::Thread thread_;
  NaiveNet::CountDownLatch latch_;
  NaiveNet::MutexLock mutex_;
  NaiveNet::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}
#endif  // NAIVENET_BASE_ASYNCLOGGING_H
