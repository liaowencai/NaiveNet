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
		// ��typedef������һ���������ͣ�����Ĵ�С����Ϊ-1�������������������ǲ���ȫ���ͣ�����׶ξͻᷢ�ִ��� 
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

