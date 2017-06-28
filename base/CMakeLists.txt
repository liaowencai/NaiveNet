# NaiveNet/base

SET(base_SRCS
	Condition.cc
	CountDownLatch.cc
	Date.cc
	Exception.cc
	FileUtil.cc
	ProcessInfo.cc
	Timestamp.cc
	TimeZone.cc
	Thread.cc
	ThreadPool.cc
	./log/LogFile.cc
	./log/Logging.cc
	./log/LogStream.cc
	)
	
# 添加log文件夹
#ADD_SUBDIRECTORY(log)

# 添加库	
ADD_LIBRARY(NaiveNet_base ${base_SRCS})
# 指定在链接目标文件时需要链接的外部库，类似于g++的-l指令
TARGET_LINK_LIBRARIES(NaiveNet_base pthread rt)

#安装目标文件到指定位置
INSTALL(TARGETS NaiveNet_base DESTINATION lib)

# 将所有匹配*.h的文件名写入变量HEADERS中
#FILE(GLOB HEADERS "*.h")
# 将HEADERS中的文件安装到指定位置
#INSTALL(FILES ${HEADERS} DESTINATION include/NaiveNet/base)	


# 添加包含测试文件的文件夹
ADD_SUBDIRECTORY(tests)
