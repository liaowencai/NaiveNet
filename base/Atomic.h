#ifndef NAIVENET_BASE_ATOMIC_H
#define NAIVENET_BASE_ATOMIC_H

#include <stdint.h>

// ԭ��������ģ�壬��������в��������̰߳�ȫ��
namespace NaiveNet
{

namespace detail
{
template<typename T>
class AtomicIntegerT // ֵ����
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
		// ����ptr��oldval��ֵ�Ƿ���ͬ�������ͬ���Ͱ�newvalд��ptr
		return __sync_val_compare_and_swap(&value_, 0, 0);
	}

	T getAndAdd(T x)
	{
		// value����x��ֵ��Ȼ�󷵻�value�ľ�ֵ
		return __sync_fetch_and_add(&value_, x);
	}

	T addAndGet(T x)
	{
		// ����getAndAdd����֤��ԭ���ԣ�Ȼ��+x�����صľ������֮���value
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

