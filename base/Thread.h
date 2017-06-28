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

	// ��ʾ�߳�ִ�еĺ�������
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

	// �߳�idֻ�����߳�������ʱ����ܱ�ȷ����������Ҫ��ThreadData::runInThread�ж�Thead���еĳ�Աtid_���޸ģ�
	// ��ˣ�tid_��Ҫ�����Ա��������ָ�����ʽ���д��ݣ�����Ӧ��ʹ��shared_ptr���й���
	// ThreadData�е�wkTid��һ��weak_ptr,��ΪTheadData����ӵ��tid_������weak_ptr����ͨ��lock()��ȡ��Ӧ��shared_ptr, ������Thread::tid_�����޸ġ�
	std::shared_ptr<pid_t> tid_;

	ThreadFunc func_;
	std::string name_;

	// ��ʾ�ڼ��δ����߳�ʵ��
	static AtomicInt32 numCreated_;
};
}

#endif // !NAIVENET_BASE_THREAD_H

