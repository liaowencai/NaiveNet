#ifndef NAIVENET_BASE_ATOMIC_H
#define NAIVENET_BASE_ATOMIC_H

#include <stdint.h>

// 原子数据类模板，该类的所有操作都是线程安全的
namespace NaiveNet
{

namespace detail
{
template<typename T>
class AtomicIntegerT // 值语义
{
public:
	AtomicIntegerT()
		: value_(0)
	{
	}

	// forbid copy and assignment
	AtomicIntegerT(const AtomicIntegerT& that) = delete;
	AtomicIntegerT& operator=(const AtomicIntegerT& that) = delete;

	T get()
	{
		// type __sync_val_compare_and_swap(type *ptr, type oldval, type newvla, ...)
		// 测试ptr与oldval的值是否相同，如果相同，就把newval写入ptr
		return __sync_val_compare_and_swap(&value_, 0, 0);
	}

	T getAndAdd(T x)
	{
		// value加上x的值，然后返回value的旧值
		return __sync_fetch_and_add(&value_, x);
	}

	T addAndGet(T x)
	{
		// 调用getAndAdd，保证了原子性，然后+x，返回的就是相加之后的value
		return getAndAdd(x) + x;
	}

	T incrementAndGet()
	{
		return addAndGet(1);
	}

	T decrementAndGet()
	{
		return addAndGet(-1);
	}

	void add(T x)
	{
		getAndAdd(x);
	}

	void increment()
	{
		incrementAndGet();
	}

	void decrement()
	{
		decrementAndGet();
	}

	T getAndSet(T newValue)
	{
		return __sync_lock_test_and_set(&value_, newValue);
	}

private:
	volatile T value_;
};
}

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif // !NAIVENET_BASE_ATOMIC_H

