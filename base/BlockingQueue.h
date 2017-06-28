#ifndef NAIVENET_BASE_BLOCKINGQUEUE_H
#define NAIVENET_BASE_BLOCKINGQUEUE_H

#include "./Condition.h"
#include "./Mutex.h"

#include <deque>
#include <assert.h>

namespace NaiveNet
{

// 在无界缓冲中，生产者不用关心仓库是否已满，只需添加数据
// 消费者在判断仓库已空时要等待生产者的信号。这时只需要用一个信号量
template <typename T>
class BlockingQueue
{
public:
	BlockingQueue(const BlockingQueue&) = delete;
	BlockingQueue& operator=(const BlockingQueue&) = delete;

public:
	BlockingQueue()
		: mutex_(),
		notEmpty_(mutex_),
		queue_()
	{
	}
	// 生产数据
	void put(const T& x)
	{
		MutexLockGuard lock(mutex_);
		queue_.push_back(x);
		notEmpty_.notify();
	}

	// 使用了移动语义-右值引用
	/*void put(t&& x)
	{
		mutexlockguard lock(mutex_);
		queue_.push_back(std::move(x));
		notempty_.notify();
	}*/
	
	// 消费数据
	T take()
	{
		MutexLockGuard lock(mutex_);
		while (queue_.empty())
		{
			// 如果仓库为空，则等待生产者信号
			notEmpty_.wait();
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	// 在take函数中判断队列是否为空时，不能使用size函数
	// 这样会造成同一线程对同一个Mutex加锁两次，造成死锁
	size_t size() const
	{
		MutexLockGuard lock(mutex_);
		return queue_.size();
	}

private:
	mutable MutexLock mutex_;
	Condition notEmpty_;
	std::deque<T> queue_;
};
}

#endif // !NAIVENET_BASE_BLOCKINGQUEUE_H

