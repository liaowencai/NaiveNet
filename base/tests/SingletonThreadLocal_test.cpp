#include "../Singleton.h"
#include "../CurrentThread.h"
#include "../ThreadLocal.h"
#include "../Thread.h"

#include <functional>
#include <stdio.h>
#include <unistd.h>
#include <string>

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

#define STL NaiveNet::Singleton<NaiveNet::ThreadLocal<Test> >::instance().value()

void print()
{
	printf("tid=%d, %p name=%s\n",
		NaiveNet::CurrentThread::tid(),
		&STL,
		STL.name().c_str());
}

void threadFunc(const char* changeTo)
{
	print();
	STL.setName(changeTo);
	sleep(1);
	print();
}

int main()
{
	STL.setName("main one");
	NaiveNet::Thread t1(std::bind(threadFunc, "thread1"));
	NaiveNet::Thread t2(std::bind(threadFunc, "thread2"));
	t1.start();
	t2.start();
	t1.join();
	print();
	t2.join();
	pthread_exit(0);
}