#ifndef NAIVENET_BASE_THREAD_H
#define NAIVENET_BASE_THREAD_H

#include "./Atomic.h"
//#include "./Types.h"

#include <functional>
#include <memory>
#include <string>
#include <pthread.h>

namespace NaiveNet
{
class Thread
{
public:
	Thread(const Thread& that) = delete;
	Thread& operator=(const Thread& that) = delete;

	// 表示线程执行的函数对象
	typedef std::function<void()> ThreadFunc;
	
	explicit Thread(const ThreadFunc&, const std::string& name = std::string());
	
	~Thread();

	void start();
	int join(); // return pthread_join()

	bool started() const { return started_; }
	pid_t tid() const { return *tid_; }
	const std::string& name() const { return name_; }
	
	static int numCreated() { return numCreated_.get(); }

private:
	void setDefaultName();

private:
	bool started_;
	bool joined_;
	pthread_t pthreadId_;

	// 线程id只有在线程启动的时候才能被确定，而且需要在ThreadData::runInThread中对Thead类中的成员tid_做修改，
	// 因此，tid_需要在类成员函数间以指针的形式进行传递，所以应当使用shared_ptr进行管理
	// ThreadData中的wkTid是一个weak_ptr,因为TheadData并不拥有tid_。但是weak_ptr可以通过lock()获取对应的shared_ptr, 进而对Thread::tid_进行修改。
	std::shared_ptr<pid_t> tid_;

	ThreadFunc func_;
	std::string name_;

	// 表示第几次创建线程实例
	static AtomicInt32 numCreated_;
};
}

#endif // !NAIVENET_BASE_THREAD_H

