#include "../ThreadLocalSingleton.h"
#include "../CurrentThread.h"
#include "../Thread.h"

#include <functional>
#include <string>
#include <stdio.h>

class Test
{
public:
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;

public:
	Test()
	{
		printf("tid=%d, constructing %p\n", NaiveNet::CurrentThread::tid(), this);
	}

	~Test()
	{
		printf("tid=%d, destructing %p %s\n", NaiveNet::CurrentThread::tid(), this, name_.c_str());
	}

	const std::string& name() const { return name_; }
	void setName(const std::string& n) { name_ = n; }

private:
	std::string name_;
};

void threadFunc(const char* changeTo)
{
	printf("tid=%d, %p name=%s\n",
		NaiveNet::CurrentThread::tid(),
		&NaiveNet::ThreadLocalSingleton<Test>::instance(),
		NaiveNet::ThreadLocalSingleton<Test>::instance().name().c_str());
	NaiveNet::ThreadLocalSingleton<Test>::instance().setName(changeTo);
	printf("tid=%d, %p name=%s\n",
		NaiveNet::CurrentThread::tid(),
		&NaiveNet::ThreadLocalSingleton<Test>::instance(),
		NaiveNet::ThreadLocalSingleton<Test>::instance().name().c_str());

	// no need to manually delete it
	// NaiveNet::ThreadLocalSingleton<Test>::destroy();
}

int main()
{
	NaiveNet::ThreadLocalSingleton<Test>::instance().setName("main one");
	NaiveNet::Thread t1(std::bind(threadFunc, "thread1"));
	NaiveNet::Thread t2(std::bind(threadFunc, "thread2"));
	t1.start();
	t2.start();
	t1.join();
	printf("tid=%d, %p name=%s\n",
		NaiveNet::CurrentThread::tid(),
		&NaiveNet::ThreadLocalSingleton<Test>::instance(),
		NaiveNet::ThreadLocalSingleton<Test>::instance().name().c_str());
	t2.join();

	pthread_exit(0);
}