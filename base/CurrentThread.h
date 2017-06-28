// Author: Wang Sheng (http://github.com/liaowencai)

#ifndef NAIVENET_BASE_CURRENTTHREAD_H
#define NAIVENET_BASE_CURRENTTHREAD_H

#include <stdint.h>

namespace NaiveNet
{
namespace CurrentThread
{
	// extern表示变量或者函数的定义在其他文件中
	// __thread表示线程私有的局部存储设施
	extern __thread	int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char* t_threadName;
	void cacheTid();

	inline int tid()
	{
		if (t_cachedTid == 0)
		{
			cacheTid();
		}
		return t_cachedTid;
	}

	inline const char* tidString() // for logging
	{
		return t_tidString;
	}

	inline int tidStringLength() // for logging
	{
		return t_tidStringLength;
	}

	inline const char* name()
	{
		return t_threadName;
	}

	bool isMainThread();

	void sleepUsec(int64_t usec);
}
}

#endif // !NAIVENET_BASE_CURRENTTHREAD_H

