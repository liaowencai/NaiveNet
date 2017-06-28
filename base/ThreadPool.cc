#include "./ThreadPool.h"
#include "./Exception.h"

#include <functional>
#include <assert.h>
#include <stdio.h>
#include <algorithm>

using namespace NaiveNet;
using namespace std::placeholders;

// 创建ThreadPool对象的线程是主线程
ThreadPool::ThreadPool(const string& name)
	: mutex_(),
	notEmpty_(mutex_),
	notFull_(mutex_),
	name_(name),
	maxQueueSize_(0),
	running_(false)
{
}

ThreadPool::~ThreadPool()
{
	// 析构函数调用stop()，先终止所有线程
	// 再销毁ThreadPool object
	if (running_)
	{
		stop();
	}
}

// 调用start之前，必须先调用setMaxQueueSize设定任务队列的大小
// 启动线程池，创建numThreads个线程
void ThreadPool::start(int numThreads)
{
	// 断言线程池为空
	assert(threads_.empty());
	running_ = true;
	// 预留空间
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i + 1);

		// 创建Thread object，设定其启动函数为runInThread
		threads_.push_back(unique_ptr<Thread>(
			new Thread(
				std::bind(&ThreadPool::runInThread, this), name_ + id)));

		// 调用pthread_create创建线程，执行入口函数runInThread
		threads_[i]->start();
	}

	// 如果线程池为空，且设置了threadInitCallback_，
	// 则调用threadInitCallback_()
	// 这相当于只有一个主线程
	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}
}

// 关闭线程池，终止所有线程
void ThreadPool::stop()
{
	{
		// 此处必须要加锁，假设没有加锁，
		// 如果工作线程在执行take时判断running_为true，进入while循环，但是还没有wait
		// 此时主线程中running_设定为false，然后notifyAll，发生在工作现场wait之前
		// 那么工作线程永远也等不到notify，将导致死锁
		MutexLockGuard lock(mutex_);
		running_ = false;

		// 唤醒所有阻塞在notEmpty_上线程
		// 也就是让所有工作线程执行完毕任务
		notEmpty_.notifyAll();
	}
	// 调用pthread_join，等待所有线程都结束
	std::for_each(threads_.begin(),
		threads_.end(),
		std::bind(&Thread::join, _1));
}

// 向任务队列中添加任务
// 客端需要执行一个任务，必须首先将该任务push进任务队列，等侯空闲线程处理 
void ThreadPool::run(const Task& task)
{
	// 如果线程池为空
	// 由主线程执行任务task()
	if (threads_.empty())
	{
		task();
	}
	else
	{

		MutexLockGuard lock(mutex_);
		while (isFull())
		{
			// 当任务队列满的时候，进行阻塞等待
			notFull_.wait();
		}
		assert(!isFull());

		// 任务队列未满，则向其添加任务
		queue_.push_back(task);
		// 唤醒工作线程来取任务
		notEmpty_.notify();
	}
}

// 取任务函数
// 线程池中的每个线程都会调用take()
ThreadPool::Task ThreadPool::take()
{
	MutexLockGuard lock(mutex_);
	while (queue_.empty() && running_)
	{
		// 任务队列为空时，阻塞等待
		notEmpty_.wait();
	}
	Task task;

	// 退出上面的while循环有可能是因为stop将running_置为false，然后notifyall
	// 此时无法保证任务队列queue_非空，因此必须加以判断
	if (!queue_.empty())
	{
		// 从任务队列头中取任务
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
			// 唤醒正在等待添加任务的线程
			notFull_.notify();
		}
	}
	return task;
}

bool ThreadPool::isFull() const
{
	// 确保调用时线程已经被锁住
	// 因此isFull调用时，必须在外部加锁
	// 调用isFull的函数run等已经做到了这一点
	mutex_.assertLocked();
	return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

// 每个线程都会运行的入口函数
void ThreadPool::runInThread()
{
	try
	{
		// 如果设置了threadInitCallback_，则进行执行任务前的一些初始化操作
		if (threadInitCallback_)
		{
			threadInitCallback_();
		}

		// 线程池处于启动状态时，一直执行任务
		// 当调用stop()函数后，running_变成了false，因而会停止执行
		while (running_)
		{
			Task task(take());
			// stop的时候，task有可能为nullptr
			if (task)
			{
				task();
			}
		}
	}
	catch (const Exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
		fprintf(stderr, "reason: %s\n", ex.what());
		abort();
	}
	catch (const std::exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
		fprintf(stderr, "reason: %s\n", ex.what());
		abort();
	}
	catch (...)
	{
		fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
		throw; // rethrow
	}
}