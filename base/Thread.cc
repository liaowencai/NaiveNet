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
	// ��ʾ�̵߳���ʵID
	__thread int t_cachedTid = 0;
	// ��char*���ͱ�ʾtid�����ڸ�ʽ�������־
	__thread char t_tidString[32];

	__thread int t_tidStringLength = 6;
	// �̵߳�����
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

// ThreadNameInitializer�������̳߳�ʼ������������ȫ�ֱ���������������Ĭ�ϵ��߳�name�������߳�id��
// ���������fork()����ô���ӽ���������afterFork��������ͬ���ĳ�ʼ��������
class ThreadNameInitializer
{
public:
	ThreadNameInitializer()
	{
		NaiveNet::CurrentThread::t_threadName = "main";
		CurrentThread::tid();

		// ���������fork()���������ӽ��̽���ִ��atFork����
		pthread_atfork(NULL, NULL, &afterFork); 
	}
};

// ȫ�ֱ�������main������ʼǰ�͹��죬���������ʼ�����ã�pthread_atfork����
// ���̵߳�����CurrenThread::t_ThreadName="main"   
ThreadNameInitializer init;

// �����࣬��¼�ˣ�
// �߳̽�Ҫִ�еĺ���func_���߳�����name_���̱߳�ʶ��wkTid_
struct ThreadData
{
	typedef NaiveNet::Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	std::string name_;

	// ThreadData�е�wkTid��һ��weak_ptr,��ΪTheadData����ӵ��tid_��
	// ����weak_ptr����ͨ��lock()��ȡ��Ӧ��shared_ptr, ������Thread::tid_�����޸ġ�
	std::weak_ptr<pid_t> wkTid_;

	ThreadData(const ThreadFunc& func,
		const std::string& name,
		// ע�⣺tid�����õ���ʽ���ݽ���
		const std::shared_ptr<pid_t>& tid)
		: func_(func),
		name_(name),
		// wkTid_�����������ü���
		wkTid_(tid)
	{ }

	void runInThread()
	{
		pid_t tid = NaiveNet::CurrentThread::tid();

		// ��weak_ptr����Ϊshared_ptr
		std::shared_ptr<pid_t> ptid = wkTid_.lock();
		if (ptid)
		{
			*ptid = tid;
			// �ͷ���ʱshared_ptr ptid�����ü���
			ptid.reset();
		}

		// prctl()�����̿��ƺ���
		NaiveNet::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
		::prctl(PR_SET_NAME, NaiveNet::CurrentThread::t_threadName);
		try
		{
			// ���õ�ֻ����һ�Σ��������Ǵ������������
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

// �����߳�
void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	// objָ���ThreadData��������
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
	// ���߳�tid���������̵�pid��ͬ
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

// Thread������ʱ��û�����ٳ��е�Pthreads���(pthread_t)��Ҳ����˵Thread����������ȴ��߳̽���;
// ���Thread����������ڳ����̣߳�Ӧ��ͨ��Thread::join()���ȴ��߳̽������ͷ��߳���Դ;
// ���Thread����������ڶ����̣߳���ô����ʱ���Զ�detach�̣߳���������Դй¶��
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

// start()����pthread_create�������߳�
// startThread�����̵߳���ں��������ε���
// startThread()->runInThread()->func_()ִ���߳��߼�
void Thread::start()
{
	assert(!started_);
	started_ = true;
	detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
	// *tid_��ֵҪ�ȵ�ִ��ThreadData::runInThread()ʱ��������ȷ��
	if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
	{
		// pthread_createִ��ʧ�ܺ�Ĵ�����
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