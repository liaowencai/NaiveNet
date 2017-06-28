#include "./ThreadPool.h"
#include "./Exception.h"

#include <functional>
#include <assert.h>
#include <stdio.h>
#include <algorithm>

using namespace NaiveNet;
using namespace std::placeholders;

// ����ThreadPool������߳������߳�
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
	// ������������stop()������ֹ�����߳�
	// ������ThreadPool object
	if (running_)
	{
		stop();
	}
}

// ����start֮ǰ�������ȵ���setMaxQueueSize�趨������еĴ�С
// �����̳߳أ�����numThreads���߳�
void ThreadPool::start(int numThreads)
{
	// �����̳߳�Ϊ��
	assert(threads_.empty());
	running_ = true;
	// Ԥ���ռ�
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		char id[32];
		snprintf(id, sizeof id, "%d", i + 1);

		// ����Thread object���趨����������ΪrunInThread
		threads_.push_back(unique_ptr<Thread>(
			new Thread(
				std::bind(&ThreadPool::runInThread, this), name_ + id)));

		// ����pthread_create�����̣߳�ִ����ں���runInThread
		threads_[i]->start();
	}

	// ����̳߳�Ϊ�գ���������threadInitCallback_��
	// �����threadInitCallback_()
	// ���൱��ֻ��һ�����߳�
	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}
}

// �ر��̳߳أ���ֹ�����߳�
void ThreadPool::stop()
{
	{
		// �˴�����Ҫ����������û�м�����
		// ��������߳���ִ��takeʱ�ж�running_Ϊtrue������whileѭ�������ǻ�û��wait
		// ��ʱ���߳���running_�趨Ϊfalse��Ȼ��notifyAll�������ڹ����ֳ�wait֮ǰ
		// ��ô�����߳���ԶҲ�Ȳ���notify������������
		MutexLockGuard lock(mutex_);
		running_ = false;

		// ��������������notEmpty_���߳�
		// Ҳ���������й����߳�ִ���������
		notEmpty_.notifyAll();
	}
	// ����pthread_join���ȴ������̶߳�����
	std::for_each(threads_.begin(),
		threads_.end(),
		std::bind(&Thread::join, _1));
}

// ������������������
// �Ͷ���Ҫִ��һ�����񣬱������Ƚ�������push��������У��Ⱥ�����̴߳��� 
void ThreadPool::run(const Task& task)
{
	// ����̳߳�Ϊ��
	// �����߳�ִ������task()
	if (threads_.empty())
	{
		task();
	}
	else
	{

		MutexLockGuard lock(mutex_);
		while (isFull())
		{
			// �������������ʱ�򣬽��������ȴ�
			notFull_.wait();
		}
		assert(!isFull());

		// �������δ�����������������
		queue_.push_back(task);
		// ���ѹ����߳���ȡ����
		notEmpty_.notify();
	}
}

// ȡ������
// �̳߳��е�ÿ���̶߳������take()
ThreadPool::Task ThreadPool::take()
{
	MutexLockGuard lock(mutex_);
	while (queue_.empty() && running_)
	{
		// �������Ϊ��ʱ�������ȴ�
		notEmpty_.wait();
	}
	Task task;

	// �˳������whileѭ���п�������Ϊstop��running_��Ϊfalse��Ȼ��notifyall
	// ��ʱ�޷���֤�������queue_�ǿգ���˱�������ж�
	if (!queue_.empty())
	{
		// ���������ͷ��ȡ����
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
			// �������ڵȴ����������߳�
			notFull_.notify();
		}
	}
	return task;
}

bool ThreadPool::isFull() const
{
	// ȷ������ʱ�߳��Ѿ�����ס
	// ���isFull����ʱ���������ⲿ����
	// ����isFull�ĺ���run���Ѿ���������һ��
	mutex_.assertLocked();
	return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

// ÿ���̶߳������е���ں���
void ThreadPool::runInThread()
{
	try
	{
		// ���������threadInitCallback_�������ִ������ǰ��һЩ��ʼ������
		if (threadInitCallback_)
		{
			threadInitCallback_();
		}

		// �̳߳ش�������״̬ʱ��һֱִ������
		// ������stop()������running_�����false�������ִֹͣ��
		while (running_)
		{
			Task task(take());
			// stop��ʱ��task�п���Ϊnullptr
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