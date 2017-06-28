#include "../ProcessInfo.h"
#include <stdio.h>

#include <inttypes.h>

int main()
{
	printf("pid = %d\n", NaiveNet::ProcessInfo::pid());
	printf("uid = %d\n", NaiveNet::ProcessInfo::uid());
	printf("euid = %d\n", NaiveNet::ProcessInfo::euid());
	printf("start time = %s\n", NaiveNet::ProcessInfo::startTime().toFormattedString().c_str());
	printf("hostname = %s\n", NaiveNet::ProcessInfo::hostname().c_str());
	printf("opened files = %d\n", NaiveNet::ProcessInfo::openedFiles());
	printf("threads = %zd\n", NaiveNet::ProcessInfo::threads().size());
	printf("num threads = %d\n", NaiveNet::ProcessInfo::numThreads());
	printf("status = %s\n", NaiveNet::ProcessInfo::procStatus().c_str());
}
