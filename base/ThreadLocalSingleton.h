#ifndef NAIVENET_BASE_THREADLOCALSINGLETON_H
#define NAIVENET_BASE_THREADLOCALSINGLETON_H

#include <assert.h>
#include <pthread.h>

namespace NaiveNet
{

template <typename T>
class ThreadLocalSingleton
{
public:
	ThreadLocalSingleton(const ThreadLocalSingleton&) = delete;
	ThreadLocalSingleton& operator=(const ThreadLocalSingleton&) = delete;

public:

	static T& instance()
	{
		if (!t_value_)
		{
			t_value_ = new T();
			deleter_.set(t_value_);
		}
		return *t_value_;
	}

	static T* pointer()
	{
		return t_value_;
	}
	
private:

	ThreadLocalSingleton();
	~ThreadLocalSingleton();

	static void destructor(void* obj)
	{
		assert(obj == t_value_);
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		T_must_be_complete_type dummy; (void)dummy;
		delete t_value_;
		t_value_ = nullptr;
	}

	class Deleter
	{
	public:
		Deleter()
		{
			pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
		}

		~Deleter()
		{
			pthread_key_delete(pkey_);
		}

		void set(T* newObj)
		{
			assert(pthread_getspecific(pkey_) == NULL);
			pthread_setspecific(pkey_, newObj);
		}

		pthread_key_t pkey_;
	};

	// 注意线程局部变量__thread关键字
	static __thread T* t_value_;

	// 静态成员只存在一个，初始化时将调用Deleter::Deleter()构造函数
	static Deleter deleter_;
};

template <typename T>
__thread T* ThreadLocalSingleton<T> ::t_value_ = nullptr;

template <typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;
}

#endif // !NAIVENET_BASE_THREADLOCALSINGLETON_H

