# NaiveNet/

cmake_minimum_required(VERSION 2.6)

project(NaiveNet CXX C)

# 启用测试
ENABLE_TESTING()

IF(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
ENDIF()

SET(CXX_FLAGS
 -g
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
-std=c++11
 -rdynamic
 )

STRING(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# 设置变量 
# 指定C++编译器为g++
SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

# 定义可执行二进制文件的位置
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# 定义库文件的输出位置
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# 指定编译过程中编译器搜索头文件的路径
# INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIRS})

STRING(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
MESSAGE(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})
#MESSAGE(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS})

# 向当前工程中添加源文件存放的子目录
ADD_SUBDIRECTORY(base)



