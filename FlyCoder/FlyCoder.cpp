#include "FlyCoder.h"


BOOL CodePageToUnicode(int iCodePage, const char szSrc[], WCHAR szDest[], int& iDestLength)
{

	int iSize = ::MultiByteToWideChar(iCodePage, 0, szSrc, -1, nullptr, 0);

	if (iSize == 0 || iSize > iDestLength || !szDest || iDestLength == 0)
	{
		iDestLength = iSize;
		return FALSE;
	}

	if (::MultiByteToWideChar(iCodePage, 0, szSrc, -1, szDest, iSize) != 0)
		iDestLength = iSize;
	else
		iDestLength = 0;

	return iDestLength != 0;
}

BOOL UnicodeToCodePage(int iCodePage, const WCHAR szSrc[], char szDest[], int& iDestLength)
{


	int iSize = ::WideCharToMultiByte(iCodePage, 0, szSrc, -1, nullptr, 0, nullptr, nullptr);

	if (iSize == 0 || iSize > iDestLength || !szDest || iDestLength == 0)
	{
		iDestLength = iSize;
		return FALSE;
	}

	if (::WideCharToMultiByte(iCodePage, 0, szSrc, -1, szDest, iSize, nullptr, nullptr) != 0)
		iDestLength = iSize;
	else
		iDestLength = 0;

	return iDestLength != 0;
}

BOOL GbkToUnicode(const char szSrc[], WCHAR szDest[], int& iDestLength)
{
	return CodePageToUnicode(CP_ACP, szSrc, szDest, iDestLength);
}

BOOL UnicodeToGbk(const WCHAR szSrc[], char szDest[], int& iDestLength)
{
	return UnicodeToCodePage(CP_ACP, szSrc, szDest, iDestLength);
}

BOOL Utf8ToUnicode(const char szSrc[], WCHAR szDest[], int& iDestLength)
{
	return CodePageToUnicode(CP_UTF8, szSrc, szDest, iDestLength);
}

BOOL UnicodeToUtf8(const WCHAR szSrc[], char szDest[], int& iDestLength)
{
	return UnicodeToCodePage(CP_UTF8, szSrc, szDest, iDestLength);
}

BOOL GbkToUtf8(const char szSrc[], char szDest[], int& iDestLength)
{
	int iMiddleLength = 0;
	GbkToUnicode(szSrc, nullptr, iMiddleLength);

	if (iMiddleLength == 0)
	{
		iDestLength = 0;
		return FALSE;
	}

	
	wchar_t* pwstr = new wchar_t[iMiddleLength + 1];

	GbkToUnicode(szSrc, pwstr, iMiddleLength);

	BOOL bRet = UnicodeToUtf8(pwstr, szDest, iDestLength);

	if (pwstr) delete[] pwstr;

	return bRet;
}

BOOL Utf8ToGbk(const char szSrc[], char szDest[], int& iDestLength)
{
	int iMiddleLength = 0;
	Utf8ToUnicode(szSrc, nullptr, iMiddleLength);

	if (iMiddleLength == 0)
	{
		iDestLength = 0;
		return FALSE;
	}


	wchar_t* pwstr = new wchar_t[iMiddleLength + 1];

	Utf8ToUnicode(szSrc, pwstr, iMiddleLength);

	BOOL bRet = UnicodeToGbk(pwstr, szDest, iDestLength);

	return bRet;
}