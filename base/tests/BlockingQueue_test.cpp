#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../Thread.h"

#include <functional>
#include <memory>
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

using NaiveNet::Thread;
using namespace std;
using namespace std::placeholders;

class Test
{
public:
	Test(int numThreads)
		: latch_(numThreads)
		// 本行必须去掉，否则会导致core
		// 2017-06-26晚上整整调试了两个小时也没有结果，看博客才知道原因：
		// vector::vector(int nums)构造函数会构造nums个vector默认元素
		// 然而threads_的元素时unique_ptr<Thread>，必须提供初始值才能构造，
		// 没有默认值
		//threads_(numThreads)
	{
		for (int i = 0; i < numThreads; ++i)
		{
			char name[32];
			snprintf(name, sizeof name, "work thread %d", i);
			threads_.push_back(unique_ptr<Thread>(
				(new Thread(
					bind(&Test::threadFunc, this), string(name)))));
		}
		
		for_each(threads_.begin(), threads_.end(), bind(&Thread::start, _1));
		/*for (int i = 0; i < numThreads; ++i)
		{
			threads_[i]->start();
		}*/
	}

	void run(int times)
	{
		printf("waiting for count down latch\n");
		latch_.wait();
		printf("all threads started\n");
		for (int i = 0; i < times; ++i)
		{
			char buf[32];
			snprintf(buf, sizeof buf, "hello %d", i);
			queue_.put(buf);
			printf("tid=%d, put data = %s, size = %zd\n", NaiveNet::CurrentThread::tid(), buf, queue_.size());
		}
	}

	void joinAll()
	{
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			queue_.put("stop"); 
		}
		for_each(threads_.begin(), threads_.end(), bind(&Thread::join, _1));
	}

private:
	void threadFunc()
	{
		printf("tid=%d, %s started\n",
			NaiveNet::CurrentThread::tid(),
			NaiveNet::CurrentThread::name());
		latch_.countDown();
		bool running = true;
		while (running)
		{
			string d(queue_.take());
			printf("tid=%d, get data = %s, size = %zd\n",
				NaiveNet::CurrentThread::tid(),
				d.c_str(),
				queue_.size());
		}

		printf("tid=%d, %s stopped\n",
			NaiveNet::CurrentThread::tid(),
			NaiveNet::CurrentThread::name());
	}

private:
	NaiveNet::BlockingQueue<string> queue_;
	NaiveNet::CountDownLatch latch_;
	vector<unique_ptr<Thread>> threads_;
};


int main()
{
	printf("pid=%d, tid=%d\n", ::getpid(), NaiveNet::CurrentThread::tid());
	Test t(5);
	t.run(100);
	t.joinAll();

	printf("number of created threads %d\n", NaiveNet::Thread::numCreated());
	return 0;
}