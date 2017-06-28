#include "./Thread.h"
#include "./CurrentThread.h"
//#include "./spdlog/spdlog.h"
#include "./log/Logging.h"
#include "./Exception.h"

#include <memory>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace NaiveNet
{
namespace CurrentThread
{
	// 表示线程的真实ID
	__thread int t_cachedTid = 0;
	// 用char*类型表示tid，便于格式化输出日志
	__thread char t_tidString[32];

	__thread int t_tidStringLength = 6;
	// 线程的名字
	__thread const char* t_threadName = "unknown";
	const bool sameType = std::is_same<int, pid_t>::value;
	static_assert(sameType, "int and pid_t are not the same type!");
}

namespace detail
{

pid_t gettid()
{
	return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
	NaiveNet::CurrentThread::t_cachedTid = 0;
	NaiveNet::CurrentThread::t_threadName = "main";
	CurrentThread::tid();
}

// ThreadNameInitializer进行主线程初始化操作（利用全局变量）：包括设置默认的线程name、缓存线程id；
// 如果进行了fork()，那么在子进程中运行afterFork函数进行同样的初始化工作。
class ThreadNameInitializer
{
public:
	ThreadNameInitializer()
	{
		NaiveNet::CurrentThread::t_threadName = "main";
		CurrentThread::tid();

		// 如果进行了fork()操作，则子进程将会执行atFork函数
		pthread_atfork(NULL, NULL, &afterFork); 
	}
};

// 全局变量，在main函数开始前就构造，完成启动初始化配置（pthread_atfork），
// 主线程的名字CurrenThread::t_ThreadName="main"   
ThreadNameInitializer init;

// 辅助类，记录了：
// 线程将要执行的函数func_，线程名字name_，线程标识符wkTid_
struct ThreadData
{
	typedef NaiveNet::Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	std::string name_;

	// ThreadData中的wkTid是一个weak_ptr,因为TheadData并不拥有tid_；
	// 但是weak_ptr可以通过lock()获取对应的shared_ptr, 进而对Thread::tid_进行修改。
	std::weak_ptr<pid_t> wkTid_;

	ThreadData(const ThreadFunc& func,
		const std::string& name,
		// 注意：tid以引用的形式传递进来
		const std::shared_ptr<pid_t>& tid)
		: func_(func),
		name_(name),
		// wkTid_不会增加引用计数
		wkTid_(tid)
	{ }

	void runInThread()
	{
		pid_t tid = NaiveNet::CurrentThread::tid();

		// 将weak_ptr提升为shared_ptr
		std::shared_ptr<pid_t> ptid = wkTid_.lock();
		if (ptid)
		{
			*ptid = tid;
			// 释放临时shared_ptr ptid的引用计数
			ptid.reset();
		}

		// prctl()：进程控制函数
		NaiveNet::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
		::prctl(PR_SET_NAME, NaiveNet::CurrentThread::t_threadName);
		try
		{
			// 有用的只有这一段，其他的是错误处理，无需理会
			func_();
			NaiveNet::CurrentThread::t_threadName = "finished";
		}
		catch (const Exception& ex)
		{
			NaiveNet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
			abort();
		}
		catch (const std::exception& ex)
		{
			NaiveNet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
			fprintf(stderr, "reason: %s\n", ex.what());
			abort();
		}
		catch (...)
		{
			NaiveNet::CurrentThread::t_threadName = "crashed";
			fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
			throw; // rethrow
		}
	}
};

// 启动线程
void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	// obj指向的ThreadData对象被销毁
	delete data;
	return NULL;
}

}
}

using namespace NaiveNet;

void CurrentThread::cacheTid()
{
	if (t_cachedTid == 0)
	{
		t_cachedTid = detail::gettid();
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
	}
}

bool CurrentThread::isMainThread()
{
	// 主线程tid和所属进程的pid相同
	return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
	::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const std::string& n)
	: started_(false),
	  joined_(false),
	  pthreadId_(0),
	  tid_(new pid_t(0)),
	  func_(func),
	  name_(n)
{
	setDefaultName();
}

// Thread析构的时候没有销毁持有的Pthreads句柄(pthread_t)，也就是说Thread的析构不会等待线程结束;
// 如果Thread对象的生命期长于线程，应当通过Thread::join()来等待线程结束并释放线程资源;
// 如果Thread对象的生命期短于线程，那么析构时会自动detach线程，避免了资源泄露。
Thread::~Thread()
{
	if (started_ && !joined_)
		pthread_detach(pthreadId_);
}

void Thread::setDefaultName()
{
	int num = numCreated_.incrementAndGet();
	if (name_.empty())
	{
		char buf[32];
		snprintf(buf, sizeof buf, "Thread%d", num);
		name_ = buf;
	}
}

// start()调用pthread_create创建新线程
// startThread是新线程的入口函数，依次调用
// startThread()->runInThread()->func_()执行线程逻辑
void Thread::start()
{
	assert(!started_);
	started_ = true;
	detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
	// *tid_的值要等到执行ThreadData::runInThread()时才能最终确定
	if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
	{
		// pthread_create执行失败后的错误处理
		started_ = false;
		delete data;
		LOG_SYSFATAL << "Failed in pthread_create";
	}
}

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}