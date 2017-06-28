#ifndef NAIVENET_BASE_BLOCKINGQUEUE_H
#define NAIVENET_BASE_BLOCKINGQUEUE_H

#include "./Condition.h"
#include "./Mutex.h"

#include <deque>
#include <assert.h>

namespace NaiveNet
{

// ���޽绺���У������߲��ù��Ĳֿ��Ƿ�������ֻ���������
// ���������жϲֿ��ѿ�ʱҪ�ȴ������ߵ��źš���ʱֻ��Ҫ��һ���ź���
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
	// ��������
	void put(const T& x)
	{
		MutexLockGuard lock(mutex_);
		queue_.push_back(x);
		notEmpty_.notify();
	}

	// ʹ�����ƶ�����-��ֵ����
	/*void put(t&& x)
	{
		mutexlockguard lock(mutex_);
		queue_.push_back(std::move(x));
		notempty_.notify();
	}*/
	
	// ��������
	T take()
	{
		MutexLockGuard lock(mutex_);
		while (queue_.empty())
		{
			// ����ֿ�Ϊ�գ���ȴ��������ź�
			notEmpty_.wait();
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	// ��take�������ж϶����Ƿ�Ϊ��ʱ������ʹ��size����
	// ���������ͬһ�̶߳�ͬһ��Mutex�������Σ��������
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

