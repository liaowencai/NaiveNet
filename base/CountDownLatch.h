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
	// mutable:具有mutable性质的类的数据成员打破了类对象的const限定，
	// 允许修改类的mutable的数据成员，即便类的其它成员仍然是const只读属性。
	mutable MutexLock mutex_;

	Condition condition_;
	int count_;
};
}

#endif // !NAIVENET_BASE_COUNTDOWNLATCH_H
