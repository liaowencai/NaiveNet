#ifndef NAIVENET_BASE_CONDITION_H
#define NAIVENET_BASE_CONDITION_H

#include "./Mutex.h"
#include <pthread.h>

namespace NaiveNet
{
class Condition
{
public:
	Condition(const Condition&) = delete;
	Condition& operator=(const Condition&) = delete;

public:
	explicit Condition(MutexLock& mutex)
		: mutex_(mutex)
	{
		MCHECK(pthread_cond_init(&pcond_, NULL));
	}

	~Condition()
	{
		MCHECK(pthread_cond_destroy(&pcond_));
	}

	void wait()
	{
		MutexLock::UnassignGuard ug(mutex_);
		MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
	}

	bool waitForSeconds(int seconds);

	// 通知某个线程
	void notify()
	{
		MCHECK(pthread_cond_signal(&pcond_));
	}

	// 通知所有等待的线程
	void notifyAll()
	{
		MCHECK(pthread_cond_broadcast(&pcond_));
	}

private:
	MutexLock& mutex_;
	pthread_cond_t pcond_;
};
}

#endif // !NAIVENET_BASE_CONDITION_H
