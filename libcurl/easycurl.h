#ifndef __easycurl_h__
#define __easycurl_h__

#include "curl.h"
#include <string>

class ReadChunk {
public:
	struct ReadStruct {
		const char* dataptr;
		long sizeleft;
	};

	ReadChunk(const char* ptr, size_t size) {
		_memchunk.dataptr = ptr;
		_memchunk.sizeleft = size;
	}

	int ReadData(void *ptr, size_t size, size_t nmemb) {
		return ReadDataCallBack(ptr, size, nmemb, &_memchunk);
	}

	static int ReadDataCallBack(void *ptr, size_t size, size_t nmemb, void *userdata) {
		ReadStruct* read_chunk = (ReadStruct*)userdata;
		int rdsize = 0;
		if(size*nmemb < 1)
			return rdsize;

		if (read_chunk->sizeleft) {
			if (read_chunk->sizeleft >= (long)nmemb)
				rdsize = nmemb;
			else
				rdsize = read_chunk->sizeleft;

			memcpy(ptr, read_chunk->dataptr, rdsize);
			read_chunk->sizeleft -= rdsize;
			return rdsize;
		}
		return 0;
	}
private:
	ReadStruct _memchunk;
};

class WriteChunk {
public:
	struct MemoryStruct {
		char* memory;
		size_t size;
	};

	WriteChunk() {
		_memchunk.memory = NULL;
		_memchunk.size = 0;
	}

	~WriteChunk() {
		clear();
	}
	
	void clear() {
		if (_memchunk.memory)
			free(_memchunk.memory);
		_memchunk.size = 0;
	}

	_inline void* get() {return (void*)&_memchunk;}
	_inline char* data() {return _memchunk.memory;}
	_inline size_t length() {return _memchunk.size;}

	static void *myrealloc(void *ptr, size_t size) {
		/* There might be a realloc() out there that doesn't like reallocing
		NULL pointers, so we take care of it here */
		if(ptr)
			return realloc(ptr, size);
		else
			return malloc(size);
	}

	size_t WriteData(void *ptr, size_t size, size_t nmemb) {
		return WriteMemoryCallback(ptr, size, nmemb, &_memchunk);
	}

	static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
		size_t realsize = size * nmemb;
		struct MemoryStruct *mem = (struct MemoryStruct *)data;
		mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
		if (mem->memory) {
			memcpy(&(mem->memory[mem->size]), ptr, realsize);
			mem->size += realsize;
			mem->memory[mem->size] = 0;
		}
		return realsize;
	}
private:
	MemoryStruct _memchunk;
};

struct EasyCurlRecver {
	class IRecverCallBack {
	public:
		virtual size_t RecvData(char *ptr, size_t size, size_t nmemb, void *userdata) = 0;
	};
	void* writethunk;
	IRecverCallBack* _callback;
};

struct EasyCurlSender {
	class IEasyCurlCallBack {
	public:
		virtual size_t SendData(void *ptr, size_t size, size_t nmemb, void *userdata) = 0;
	};
	void* readthunk;
	IEasyCurlCallBack* _callback;
};

class EasyCurl {
public:
	EasyCurl();
	~EasyCurl();
	bool Init();
	void UnInit();
	void SetRecver(EasyCurlRecver* recver);
	void SetSender(EasyCurlSender* sender);
	CURLcode SetURL(const char* url); 
	CURLcode SetTimeOut(int second);
	CURLcode SetJsonTimeOut(int second);
	CURLcode SetReferer(const char* referer);
	CURLcode SetUserAgent(const char* agent);
	CURLcode SetFileHandle(FILE* file);
	CURLcode SetPostFieldsize(size_t size);
	CURLcode SetCookie(const char* cookie);
	CURLcode EnableFollowLocation(bool enable);
	CURLcode EnableProgress(bool enable);
	CURLcode EnablePost(bool enable);

	CURLcode Perform();
	CURL* GetHandle() const;
protected:
	//CURLcode SetWriteData(void* data);
	static size_t Writer(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t Reader(void *ptr, size_t size, size_t nmemb, void *userdata);
private:
	CURL* _curl_handle;
	EasyCurlRecver* _recver;
	EasyCurlSender* _sender;
};

//////////////////////////////////////////////////////////////////////////
//
class EasyCurlGet  {
public:
	virtual bool Get(const char* url);
	const char* GetData() {return _date.data();}
	size_t DataSize() {return _date.length();}
private:
	EasyCurl _curl;
	WriteChunk _date;
};

class EasyCurlDownload:
	public EasyCurlRecver::IRecverCallBack {
public:
	BOOL DownLoad2File(const char* url, const wchar_t* savefile);
protected:
	size_t RecvData(char *ptr, size_t size, size_t nmemb, void *userdata);
private:
	EasyCurl _curl;
};
//////////////////////////////////////////////////////////////////////////
//
class EasyCurlPost {
public:
	virtual bool Post(const char* url, void* postdata, size_t datasize);
	virtual bool PostJson(const char* url, void* postdata, size_t datasize);
	const char* GetData() {return _date.data();}
	size_t DataSize() {return _date.length();}
private:
	EasyCurl _curl;
	WriteChunk _date;
};


#endif