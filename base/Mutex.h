#ifndef NAIVENET_BASE_MUTEX_H
#define NAIVENET_BASE_MUTEX_H

#include "./CurrentThread.h"
#include <assert.h>
#include <pthread.h>

//检查pthread系列函数的返回值，成功返回0
#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum,
	const char *file,
	unsigned int line,
	const char *function)
	__THROW __attribute__((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace NaiveNet
{
// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_; // GUARDED BY mutex_
// };

class MutexLock
{
public:
	MutexLock(const MutexLock&) = delete;
	MutexLock& operator=(const MutexLock&) = delete;

public:
	MutexLock()
		: holder_(0)
	{
		MCHECK(pthread_mutex_init(&mutex_, NULL));
	}

	~MutexLock()
	{
		assert(holder_ == 0);
		MCHECK(pthread_mutex_destroy(&mutex_));
	}

	bool isLockedByThisThread() const
	{
		return holder_ == CurrentThread::tid();
	}

	void assertLocked() const
	{
		assert(isLockedByThisThread());
	}

	// only for internal usage
	void lock()
	{
		MCHECK(pthread_mutex_lock(&mutex_));
		assignHolder();
	}
	void unlock()
	{
		unassignHolder();
		MCHECK(pthread_mutex_unlock(&mutex_));
	}

	// 返回内部拥有的mutex指针，供condition的wait函数使用
	pthread_mutex_t* getPthreadMutex()
	{
		return &mutex_;
	}

private:
	friend class Condition;

	// 嵌套类
	class UnassignGuard
	{
	public:
		UnassignGuard(const UnassignGuard&) = delete;
		UnassignGuard& operator=(const UnassignGuard&) = delete;

	public:
		UnassignGuard(MutexLock& owner)
			: owner_(owner)
		{
			owner_.unassignHolder();
		}

		~UnassignGuard()
		{
			owner_.assignHolder();
		}

	private:
		MutexLock& owner_;
	};

	void unassignHolder()
	{
		holder_ = 0;
	}

	void assignHolder()
	{
		holder_ = CurrentThread::tid();
	}

private:
	pthread_mutex_t mutex_; // 实际的mutex句柄
	pid_t holder_; // 占有这把锁的thread
};

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
// 一个RAII类，实现scoped-lock
class MutexLockGuard
{
public:
	MutexLockGuard(const MutexLockGuard&) = delete;
	MutexLockGuard& operator=(const MutexLockGuard&) = delete;

public:
	explicit MutexLockGuard(MutexLock& mutex)
		: mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

private:
	// 注意：此处的数据成员是引用，保证整个过程中mutex有且只有一个
	MutexLock& mutex_;
};
}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
// 防止错误使用这个类 例如MutexLockGuard(mutex_);
// 此时加锁返回仅限于这一行
#define MutexLockGuard(x) error "Missing guard object name"

#endif // !NAIVENET_BASE_MUTEX_H
