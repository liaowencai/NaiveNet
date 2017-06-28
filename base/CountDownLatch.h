#ifndef NAIVENET_BASE_COUNTDOWNLATCH_H
#define NAIVENET_BASE_COUNTDOWNLATCH_H

#include "./Condition.h"
#include "./Mutex.h"

namespace NaiveNet
{
class CountDownLatch
{
public:
	CountDownLatch(const CountDownLatch&) = delete;
	CountDownLatch& operator=(const CountDownLatch&) = delete;

public:
	explicit CountDownLatch(int count);

	void wait();
	
	void countDown();

	int getCount() const;

private:
	// mutable:����mutable���ʵ�������ݳ�Ա������������const�޶���
	// �����޸����mutable�����ݳ�Ա���������������Ա��Ȼ��constֻ�����ԡ�
	mutable MutexLock mutex_;

	Condition condition_;
	int count_;
};
}

#endif // !NAIVENET_BASE_COUNTDOWNLATCH_H
