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
	// ���̳߳ؿ�ʼ����֮ǰ����Ҫ������������еĴ�С������setMaxQueueSize()������Ϊ�����̳߳�ʱ���̻߳���������ȡ����
	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }

	void setThreadInitCallback(const Task& cb)
	{
		threadInitCallback_ = cb;
	}

	// �����̳߳أ�����numThreads���߳�
	void start(int numThreads);
	// �ر��̳߳أ���ÿ���߳��������pthread_join
	void stop();

	// ������������������
	// could block if maxQueueSize > 0
	void run(const Task& f);

private:
	bool isFull() const;

	// �̳߳����߳̽�Ҫִ�еĺ���
	// ���ȵ��ó�ʼ������
	// Ȼ������������ȡ�������ټ���ִ��
	void runInThread();

	// �����������ȡ������
	Task take();

	MutexLock mutex_;

	// �������queue_��Ϊ���ˣ����������ִ���ˣ��������ѵȴ����߳�
	Condition notEmpty_;
	// �������queue_�����ˣ��пռ����ʹ���ˣ��������ѵȴ����߳�
	Condition notFull_;

	string name_;

	// threadInitCallback_����setThreadInitCallback(const Task& cb)���ã����ûص�������ÿ����ִ������ǰ�ȵ��á�
	Task threadInitCallback_;

	// �̳߳�threads_�����͵�������-������ģ��
	vector<unique_ptr<NaiveNet::Thread>> threads_;
	// �������
	deque<Task> queue_;
	// �����������
	size_t maxQueueSize_;
	bool running_;	
};
}

#endif // !NAIVENET_BASE_THREADPOOL_H

