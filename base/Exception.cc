// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "./Exception.h"

#include <execinfo.h>
#include <stdlib.h>

using namespace NaiveNet;

// �쳣�ڹ����ʱ���Ҫ�������trace
Exception::Exception(const char* msg)
	: message_(msg)
{
	fillStackTrace();
}

Exception::Exception(const std::string& msg)
	: message_(msg)
{
	fillStackTrace();
}

Exception::~Exception() throw ()
{
}

const char* Exception::what() const throw()
{
	return message_.c_str();
}

const char* Exception::stackTrace() const throw()
{
	return stack_.c_str();
}

//���ջ�ۼ��������漰����������
// #include <execinfo.h>
// int backtrace(void **buffer, int size);
// char **backtrace_symbols(void *const *buffer, int size);
// void backtrace_symbols_fd(void *const *buffer, int size, int fd);

void Exception::fillStackTrace()
{
	const int len = 200;
	void* buffer[len];
	int nptrs = ::backtrace(buffer, len);
	char** strings = ::backtrace_symbols(buffer, nptrs);
	if (strings)
	{
		for (int i = 0; i < nptrs; ++i)
		{
			// TODO demangle funcion name with abi::__cxa_demangle
			// C++�еĺ��������������ָı࣬�������л�ԭ������ʹ��abi::__cxa_demangle
			// �ο����ļ���ĩβ
			stack_.append(strings[i]);
			stack_.push_back('\n');
		}
		free(strings);
	}
}

// �����ṩһ�����Խ�ջ�ۼ��б��ı�ĺ������ֽ��л�ԭ�ĺ�����
// //�����ָı�
// string Exception::demangle(const char* symbol)
// {
//     size_t size;
//     int status;
//     char temp[128];
//     char* demangled;
//     //first, try to demangle a c++ name
//     if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp)) {
//         if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status))) {
//           string result(demangled);
//           free(demangled);
//           return result;
//         }
//     }
//     //if that didn't work, try to get a regular c symbol
//     if (1 == sscanf(symbol, "%127s", temp)) {
//         return temp;
//     }

//     //if all else fails, just return the symbol
//     return symbol;
// }

