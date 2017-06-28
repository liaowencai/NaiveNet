# NaiveNet/base/tests

# 使用给定的源文件Atomic_unittest.cpp，
# 为工程引入一个可执行文件atomic_unittest
ADD_EXECUTABLE(atomic_unittest Atomic_unittest.cpp)
# 增加测试用例
ADD_TEST(NAME atomic_unittest COMMAND atomic_unittest)

ADD_EXECUTABLE(date_unittest Date_unittest.cpp)
TARGET_LINK_LIBRARIES(date_unittest NaiveNet_base)
ADD_TEST(NAME date_unittest COMMAND date_unittest)

ADD_EXECUTABLE(timestamp_unittest Timestamp_unittest.cpp)
TARGET_LINK_LIBRARIES(timestamp_unittest NaiveNet_base)
ADD_TEST(NAME timestamp_unittest COMMAND timestamp_unittest)

ADD_EXECUTABLE(timezone_unittest TimeZone_unittest.cpp)
TARGET_LINK_LIBRARIES(timezone_unittest NaiveNet_base)
ADD_TEST(NAME timezone_unittest COMMAND timezone_unittest)

ADD_EXECUTABLE(logfile_test LogFile_test.cpp)
TARGET_LINK_LIBRARIES(logfile_test NaiveNet_base)

#ADD_EXECUTABLE(logging_test Logging_test.cpp)
#TARGET_LINK_LIBRARIES(logging_test NaiveNet_base)

ADD_EXECUTABLE(logstream_bench LogStream_bench.cpp)
TARGET_LINK_LIBRARIES(logstream_bench NaiveNet_base)

ADD_EXECUTABLE(processinfo_test ProcessInfo_test.cpp)
TARGET_LINK_LIBRARIES(processinfo_test NaiveNet_base)

ADD_EXECUTABLE(fileutil_test FileUtil_test.cpp)
TARGET_LINK_LIBRARIES(fileutil_test NaiveNet_base)
ADD_TEST(NAME fileutil_test COMMAND fileutil_test)

ADD_EXECUTABLE(fork_test Fork_test.cpp)
TARGET_LINK_LIBRARIES(fork_test NaiveNet_base)

ADD_EXECUTABLE(thread_bench Thread_bench.cpp)
TARGET_LINK_LIBRARIES(thread_bench NaiveNet_base)

ADD_EXECUTABLE(thread_test Thread_test.cpp)
TARGET_LINK_LIBRARIES(thread_test NaiveNet_base)

ADD_EXECUTABLE(singleton_test Singleton_test.cpp)
TARGET_LINK_LIBRARIES(singleton_test NaiveNet_base)

ADD_EXECUTABLE(singleton_threadlocal_test SingletonThreadLocal_test.cpp)
TARGET_LINK_LIBRARIES(singleton_threadlocal_test NaiveNet_base)

ADD_EXECUTABLE(mutex_test Mutex_test.cpp)
TARGET_LINK_LIBRARIES(mutex_test NaiveNet_base)

ADD_EXECUTABLE(blockingqueue_test BlockingQueue_test.cpp)
TARGET_LINK_LIBRARIES(blockingqueue_test NaiveNet_base)

ADD_EXECUTABLE(threadlocal_test ThreadLocal_test.cpp)
TARGET_LINK_LIBRARIES(threadlocal_test NaiveNet_base)

ADD_EXECUTABLE(threadlocalsingleton_test ThreadLocalSingleton_test.cpp)
TARGET_LINK_LIBRARIES(threadlocalsingleton_test NaiveNet_base)

ADD_EXECUTABLE(threadpool_test ThreadPool_test.cpp)
TARGET_LINK_LIBRARIES(threadpool_test NaiveNet_base)
