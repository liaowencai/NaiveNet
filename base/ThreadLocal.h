#ifndef NAIVENET_BASE_THREADLOCAL_H
#define NAIVENET_BASE_THREADLOCAL_H

#include "./Mutex.h"
#include <pthread.h>


namespace NaiveNet
{
template <typename T>
class ThreadLocal
{
public:
	ThreadLocal(const ThreadLocal&) = delete;
	ThreadLocal& operator=(const ThreadLocal&) = delete;

public:
	ThreadLocal()
	{
		MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
	}

	~ThreadLocal()
	{
		MCHECK(pthread_key_delete(pkey_));
	}

	T& value()
	{
		T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
		if (!perThreadValue)
		{
			T* newObj = new T();
			MCHECK(pthread_setspecific(pkey_, newObj));
			perThreadValue = newObj;
		}
		return *perThreadValue;
	}

private:
	// 注意：destructor是类的静态成员函数
	static void destructor(void* x)
	{
		T* obj = static_cast<T*>(x);

		// 检测是否是完全类型
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		T_must_be_complete_type dummy;
		(void)dummy;
		delete obj;
	}

private:
	pthread_key_t pkey_;
};
}

#endif // !NAIVENET_BASE_THREADLOCAL_H

