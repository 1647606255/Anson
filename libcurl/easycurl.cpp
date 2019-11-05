#include <afxwin.h>
#include "easycurl.h"
#include <assert.h>

EasyCurl::EasyCurl():
	_curl_handle(NULL),
	_recver(NULL),
	_sender(NULL) {
}

EasyCurl::~EasyCurl() {
	UnInit();
}

bool EasyCurl::Init() {
	curl_global_init(CURL_GLOBAL_WIN32);
	if (!_curl_handle)
		_curl_handle = curl_easy_init();
	return _curl_handle != NULL;
}

void EasyCurl::UnInit() {
	if (_curl_handle) {
		curl_easy_cleanup(_curl_handle);
		curl_global_cleanup();
		_curl_handle = NULL;
	}
}

void EasyCurl::SetRecver(EasyCurlRecver* recver) {
	_recver = recver;
	curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(_curl_handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(_curl_handle, CURLOPT_WRITEFUNCTION, Writer);
}

void EasyCurl::SetSender(EasyCurlSender* sender) {
	_sender = sender;

	curl_easy_setopt(_curl_handle, CURLOPT_READDATA, this);
	curl_easy_setopt(_curl_handle, CURLOPT_READFUNCTION, Reader);
}

CURLcode EasyCurl::SetURL(const char* url) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_URL, url);
}

CURLcode EasyCurl::SetTimeOut(int second) {
	assert(_curl_handle);
	struct curl_slist *headers = NULL;
	//headers = curl_slist_append(headers, "Accept: application/json");  
    headers = curl_slist_append(headers, "Content-Type:text/xml");  
    headers = curl_slist_append(headers, "charset=utf-8");  
    //headers = curl_slist_append(headers, tmp);
	curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, headers);
	return curl_easy_setopt(_curl_handle, CURLOPT_TIMEOUT, second);
}
CURLcode EasyCurl::SetJsonTimeOut(int second) {
	assert(_curl_handle);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");  
   // headers = curl_slist_append(headers, "Content-Type:text/xml");  
    headers = curl_slist_append(headers, "charset=utf-8");  
    //headers = curl_slist_append(headers, tmp);
	curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, headers);
	return curl_easy_setopt(_curl_handle, CURLOPT_TIMEOUT, second);
}
CURLcode EasyCurl::SetReferer(const char* referer) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_REFERER, referer);
}

CURLcode EasyCurl::SetUserAgent(const char* agent) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_USERAGENT, agent);
}

CURLcode EasyCurl::SetFileHandle(FILE* file) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_FILE, file);
}

CURLcode EasyCurl::SetCookie(const char* cookie) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_COOKIE, cookie);
}

CURLcode EasyCurl::SetPostFieldsize(size_t size) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDSIZE, size);
}

CURLcode EasyCurl::EnableFollowLocation(bool enable) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_FOLLOWLOCATION, enable?1:0);
}

CURLcode EasyCurl::EnableProgress(bool enable) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_NOPROGRESS, enable?0:1);
}

CURLcode EasyCurl::EnablePost(bool enable) {
	assert(_curl_handle);
	return curl_easy_setopt(_curl_handle, CURLOPT_POST, enable?1:0);
}

CURLcode EasyCurl::Perform() {
	return curl_easy_perform(_curl_handle);
}

size_t EasyCurl::Writer(char *ptr, size_t size, size_t nmemb, void *userdata) {
	EasyCurl* curl = (EasyCurl*)userdata;
	if (curl->_recver) {
		if (curl->_recver->_callback) {
			return curl->_recver->_callback->RecvData(ptr, size, nmemb, curl->_recver->writethunk);
		} else {
			//
			WriteChunk* writethunk = (WriteChunk*)curl->_recver->writethunk;
			return writethunk->WriteData(ptr, size, nmemb);
		}
	}
	return 0;
}

size_t EasyCurl::Reader(void *ptr, size_t size, size_t nmemb, void *userdata) {
	EasyCurl* curl = (EasyCurl*)userdata;
	if (curl->_sender) {
		if (curl->_sender->_callback) {
			return curl->_sender->_callback->SendData(ptr, size, nmemb, curl->_sender->readthunk);
		} else {
			//
			ReadChunk* readthunk = (ReadChunk*)curl->_sender->readthunk;
			return readthunk->ReadData(ptr, size, nmemb);
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// EasyCurlGet

bool EasyCurlGet::Get(const char* url) {
	CURLcode code;
	if (_curl.Init()) {
		_date.clear();
		_curl.SetURL(url);
		EasyCurlRecver recver;
		recver._callback = NULL;
		recver.writethunk = &_date;
		_curl.SetRecver(&recver);
		_curl.SetTimeOut(35);
		_curl.EnableFollowLocation(true);
		code = _curl.Perform();
	}
	return code == CURLE_OK;
}

BOOL EasyCurlDownload::DownLoad2File(const char *url, const wchar_t *savefile) {
	CURLcode code = CURLE_FAILED_INIT;
	if (_curl.Init()) {
		_curl.SetURL(url);
		EasyCurlRecver recver;
		FILE* stream =_wfopen(savefile, L"wb");
		if (stream) {
			recver._callback = this;
			recver.writethunk = stream;
			_curl.SetFileHandle(stream);
			_curl.SetRecver(&recver);
			_curl.EnableProgress(0);
			_curl.SetTimeOut(5);
			_curl.EnableFollowLocation(true);
			code = _curl.Perform();
			fclose(stream);
		}
	}
	return code == CURLE_OK;
}

size_t EasyCurlDownload::RecvData(char *ptr, size_t size, size_t nmemb, void *userdata) {
	size_t written = fwrite(ptr, size, nmemb, (FILE *)userdata);
	return written;
}

//////////////////////////////////////////////////////////////////////////
// EasyCurlPost
bool EasyCurlPost::Post(const char* url, void* postdata, size_t datasize) {
	CURLcode code = CURLE_FAILED_INIT;
	if (_curl.Init()) {
		_curl.SetTimeOut(35);
		_curl.SetURL(url);
		_curl.EnablePost(true);

		ReadChunk read_chunk((const char*)postdata, datasize);

		EasyCurlSender sender;
		sender._callback = NULL;
		sender.readthunk = &read_chunk;
		_curl.SetSender(&sender);
		_curl.SetPostFieldsize(datasize);

		_date.clear();
		EasyCurlRecver recver;
		recver._callback = NULL;
		recver.writethunk = &_date;
		_curl.SetRecver(&recver);
		code =_curl.Perform();
	}
	return code ==  CURLE_OK;
}
//////////////////////////////////////////////////////////////////////////
// EasyCurlPost
bool EasyCurlPost::PostJson(const char* url, void* postdata, size_t datasize) {
	CURLcode code = CURLE_FAILED_INIT;
	if (_curl.Init()) {
		_curl.SetJsonTimeOut(10);
		_curl.SetURL(url);
		_curl.EnablePost(true);
		
		ReadChunk read_chunk((const char*)postdata, datasize);
		
		EasyCurlSender sender;
		sender._callback = NULL;
		sender.readthunk = &read_chunk;
		_curl.SetSender(&sender);
		_curl.SetPostFieldsize(datasize);
		
		_date.clear();
		EasyCurlRecver recver;
		recver._callback = NULL;
		recver.writethunk = &_date;
		_curl.SetRecver(&recver);
		code =_curl.Perform();
	}
	return code ==  CURLE_OK;
}