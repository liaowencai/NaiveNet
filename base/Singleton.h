#ifndef NAIVENET_BASE_SINGLETON_H
#define NAIVENET_BASE_SINGLETON_H

#include <pthread.h>
#include <stdlib.h>

namespace NaiveNet
{

namespace detail
{	
template <typename T>
struct has_no_destroy
{
	template <typename C> static char test(decltype(&C::no_destroy)); // or decltype in C++11
	template <typename C> static int32_t test(...);
	const static bool value = sizeof(test<T>(0)) == 1;
};
}


template <typename T>
class Singleton
{
public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

public:
	static T& instance()
	{
		pthread_once(&ponce_, &Singleton::init);
		return *value_;
	}

private:
	Singleton();
	~Singleton();

	static void init()
	{
		value_ = new T();
		if (!detail::has_no_destroy<T>::value)
		{
			::atexit(destroy);
		}
	}

	static void destroy()
	{
		// 用typedef定义了一个数组类型，数组的大小不能为-1，利用这个方法，如果是不完全类型，编译阶段就会发现错误 
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		T_must_be_complete_type dummy; (void)dummy;

		delete value_;
	}

private:
	static pthread_once_t ponce_;
	static T* value_;
};

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::value_ = nullptr;

}
#endif // !NAIVENET_BASE_SINGLETON_H

