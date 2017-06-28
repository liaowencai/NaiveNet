#include "../ThreadPool.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"
#include "../log/Logging.h"

#include <functional>
#include <stdio.h>
#include <string>

#include <unistd.h>

void print()
{
	printf("tid=%d\n", NaiveNet::CurrentThread::tid());
}

void printString(const std::string& str)
{
	LOG_INFO << str;
	usleep(100 * 1000);
}

void test(int maxSize)
{
	LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
	NaiveNet::ThreadPool pool("MainThreadPool");
	pool.setMaxQueueSize(maxSize);
	pool.start(5);

	LOG_WARN << "Adding";
	pool.run(print);
	pool.run(print);
	for (int i = 0; i < 100; ++i)
	{
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", i);
		pool.run(std::bind(printString, std::string(buf)));
	}
	LOG_WARN << "Done";

	NaiveNet::CountDownLatch latch(1);
	pool.run(std::bind(&NaiveNet::CountDownLatch::countDown, &latch));
	latch.wait();
	pool.stop();
}

int main()
{
	test(0);
	test(1);
	test(5);
	test(10);
	test(50);
}