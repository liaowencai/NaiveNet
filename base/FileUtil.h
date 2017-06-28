#ifndef NAIVENET_BASE_FILEUTIL_H
#define NAIVENET_BASE_FILEUTIL_H

#include "./StringPiece.h"
#include <stdint.h>

namespace NaiveNet
{

namespace FileUtil
{
// read small file < 64KB
class ReadSmallFile
{
public:
	ReadSmallFile(const ReadSmallFile&) = delete;
	ReadSmallFile& operator=(const ReadSmallFile&) = delete;

public:
	ReadSmallFile(StringArg filename);
	~ReadSmallFile();

	// return errno
	template <typename String>
	int readToString(int maxSize,
		String* content,
		int64_t* fileSize,
		int64_t* modifyTime,
		int64_t* createTime);

	// read at maxium kBufferSize into buf_
	int readToBuffer(int* size);

	const char* buffer() const { return buf_; }

	static const int kBufferSize = 64 * 1024;

private:
	int fd_;
	int err_;
	char buf_[kBufferSize];
};

// read the file content, returns errno if error occurs
template <typename String>
int readFile(StringArg filename,
	int maxSize,
	String* content,
	int64_t* fileSize = nullptr,
	int64_t* modifyTime = nullptr,
	int64_t* createTime = nullptr)
{
	ReadSmallFile file(filename);
	return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe
class AppendFile
{
public:
	AppendFile(const AppendFile&) = delete;
	AppendFile& operator=(const AppendFile&) = delete;

public:
	explicit AppendFile(StringArg filename);

	~AppendFile();
	
	void append(const char* logline, const size_t len);

	void flush();

	size_t writtenBytes() const { return writtenBytes_; }

private:
	size_t write(const char* logline, size_t len);

	FILE* fp_;
	char buffer_[64 * 1024];
	size_t writtenBytes_;
};

}

}

#endif // !NAIVENET_BASE_FILEUTIL_H

