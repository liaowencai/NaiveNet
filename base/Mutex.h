#ifndef NAIVENET_BASE_MUTEX_H
#define NAIVENET_BASE_MUTEX_H

#include "./CurrentThread.h"
#include <assert.h>
#include <pthread.h>

//���pthreadϵ�к����ķ���ֵ���ɹ�����0
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

	// �����ڲ�ӵ�е�mutexָ�룬��condition��wait����ʹ��
	pthread_mutex_t* getPthreadMutex()
	{
		return &mutex_;
	}

private:
	friend class Condition;

	// Ƕ����
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
	pthread_mutex_t mutex_; // ʵ�ʵ�mutex���
	pid_t holder_; // ռ���������thread
};

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
// һ��RAII�࣬ʵ��scoped-lock
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
	// ע�⣺�˴������ݳ�Ա�����ã���֤����������mutex����ֻ��һ��
	MutexLock& mutex_;
};
}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
// ��ֹ����ʹ������� ����MutexLockGuard(mutex_);
// ��ʱ�������ؽ�������һ��
#define MutexLockGuard(x) error "Missing guard object name"

#endif // !NAIVENET_BASE_MUTEX_H
