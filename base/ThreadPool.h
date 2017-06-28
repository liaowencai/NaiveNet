#ifndef NAIVENET_BASE_THREADPOOL_H
#define NAIVENET_BASE_THREADPOOL_H

#include "./Condition.h"
#include "./Mutex.h"
#include "./Thread.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <deque>

namespace NaiveNet
{

using std::string;
using std::deque;
using std::vector;
using std::unique_ptr;

class ThreadPool
{
public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

public:
	typedef std::function<void()> Task;

	explicit ThreadPool(const string& name = string("ThreadPool"));
	~ThreadPool();

	// Must be called before start()
	// 在线程池开始运行之前，需要先设置任务队列的大小（调用setMaxQueueSize()），因为运行线程池时，线程会从任务队列取任务
	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }

	void setThreadInitCallback(const Task& cb)
	{
		threadInitCallback_ = cb;
	}

	// 启动线程池，创建numThreads个线程
	void start(int numThreads);
	// 关闭线程池，对每个线程逐个调用pthread_join
	void stop();

	// 向任务队列中添加任务
	// could block if maxQueueSize > 0
	void run(const Task& f);

private:
	bool isFull() const;

	// 线程池中线程将要执行的函数
	// 首先调用初始化函数
	// 然后从任务队列中取出任务，再加以执行
	void runInThread();

	// 从任务队列中取出任务
	Task take();

	MutexLock mutex_;

	// 任务队列queue_不为空了，有任务可以执行了，进而唤醒等待的线程
	Condition notEmpty_;
	// 任务队列queue_不满了，有空间可以使用了，进而唤醒等待的线程
	Condition notFull_;

	string name_;

	// threadInitCallback_可由setThreadInitCallback(const Task& cb)设置，设置回调函数，每次在执行任务前先调用。
	Task threadInitCallback_;

	// 线程池threads_：典型的生产者-消费者模型
	vector<unique_ptr<NaiveNet::Thread>> threads_;
	// 任务队列
	deque<Task> queue_;
	// 任务队列容量
	size_t maxQueueSize_;
	bool running_;	
};
}

#endif // !NAIVENET_BASE_THREADPOOL_H

