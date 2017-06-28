#include "./Condition.h"

#include <errno.h>

bool NaiveNet::Condition::waitForSeconds(int seconds)
{
	struct timespec abstime;
	// FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
	clock_gettime(CLOCK_REALTIME, &abstime); //获取当前的时间
	abstime.tv_sec += seconds;
	MutexLock::UnassignGuard ug(mutex_);
	return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}