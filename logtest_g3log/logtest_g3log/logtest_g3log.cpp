// logtest_g3log.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"
#include "MyCustFileSink.h"
#include "coredump/MiniDumper.h"
//#include "StackWalker/StackWalker.h"

#include <windows.h>
#include<stdio.h>
#include <iostream>
#include <memory>

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
const std::string path_to_log_file = "./";
#else
const std::string path_to_log_file = "/tmp/";
#endif

#define UNHANDLED_EXCEPTION_TEST

#if _MSC_VER < 1400
#define _tcscpy_s _tcscpy
#define _tcscat_s _tcscat
#define _stprintf_s _stprintf
#define strcpy_s(a, b, c) strcpy(a, c)
#endif



//void breakHere() {
//	std::ostringstream oss;
//	oss << "Fatal hook function: " << __FUNCTION__ << ":" << __LINE__ << " was called";
//	oss << " through g3::setFatalPreLoggingHook(). setFatalPreLoggingHook should be called AFTER g3::initializeLogging()" << std::endl;
//	LOG(G3LOG_DEBUG) << oss.str();
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
//	__debugbreak();
//#endif
//}

void funcTest()
{
	char test[5] = { 0 };
	test[5] = '0';
}


#ifdef UNHANDLED_EXCEPTION_TEST

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole : public StackWalker
{
protected:
	virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

// For more info about "PreventSetUnhandledExceptionFilter" see:
// "SetUnhandledExceptionFilter" and VC8
// http://blog.kalmbachnet.de/?postid=75
// and
// Unhandled exceptions in VC8 and above?for x86 and x64
// http://blog.kalmbach-software.de/2008/04/02/unhandled-exceptions-in-vc8-and-above-for-x86-and-x64/
// Even better: http://blog.kalmbach-software.de/2013/05/23/improvedpreventsetunhandledexceptionfilter/

#if defined(_M_X64) || defined(_M_IX86)
static BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL)
		return FALSE;
	void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL)
		return FALSE;

#ifdef _M_IX86
	// Code for x86:
	// 33 C0                xor         eax,eax
	// C2 04 00             ret         4
	unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
	// 33 C0                xor         eax,eax
	// C3                   ret
	unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#else
#error "The following code only works for x86 and x64!"
#endif

	DWORD dwOldProtect = 0;
	BOOL  bProt = VirtualProtect(pOrgEntry, sizeof(szExecute), PAGE_EXECUTE_READWRITE, &dwOldProtect);

	SIZE_T bytesWritten = 0;
	BOOL   bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, szExecute, sizeof(szExecute),
		&bytesWritten);

	if ((bProt != FALSE) && (dwOldProtect != PAGE_EXECUTE_READWRITE))
	{
		DWORD dwBuf;
		VirtualProtect(pOrgEntry, sizeof(szExecute), dwOldProtect, &dwBuf);
	}
	return bRet;
}
#else
#pragma message("This code works only for x86 and x64!")
#endif

static TCHAR s_szExceptionLogFileName[_MAX_PATH] = _T("\\exceptions.log"); // default
static BOOL  s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
#ifdef _M_IX86
	if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
	{
		static char MyStack[1024 * 128]; // be sure that we have enough space...
		// it assumes that DS and SS are the same!!! (this is the case for Win32)
		// change the stack only if the selectors are the same (this is the case for Win32)
		//__asm push offset MyStack[1024*128];
		//__asm pop esp;
		__asm mov eax, offset MyStack[1024 * 128];
		__asm mov esp, eax;
	}
#endif

	

	StackWalkerToConsole sw; // output to console
	sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
	TCHAR lString[500];
	_stprintf_s(lString,
		_T("*** Unhandled Exception! See console output for more infos!\n")
		_T("   ExpCode: 0x%8.8X\n")
		_T("   ExpFlags: %d\n")
#if _MSC_VER >= 1900
		_T("   ExpAddress: 0x%8.8p\n")
#else
		_T("   ExpAddress: 0x%8.8X\n")
#endif
		_T("   Please report!"),
		pExPtrs->ExceptionRecord->ExceptionCode, pExPtrs->ExceptionRecord->ExceptionFlags,
		pExPtrs->ExceptionRecord->ExceptionAddress);
	FatalAppExit(-1, lString);
	return EXCEPTION_CONTINUE_SEARCH;
}

static void InitUnhandledExceptionFilter()
{
	TCHAR szModName[_MAX_PATH];
	if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0)
	{
		_tcscpy_s(s_szExceptionLogFileName, szModName);
		_tcscat_s(s_szExceptionLogFileName, _T(".exp.log"));
	}
	if (s_bUnhandledExeptionFilterSet == FALSE)
	{
		// set global exception handler (for handling all unhandled exceptions)
		SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
#if defined _M_X64 || defined _M_IX86
		PreventSetUnhandledExceptionFilter();
#endif
		s_bUnhandledExeptionFilterSet = TRUE;
	}
}
#endif // UNHANDLED_EXCEPTION_TEST


CMiniDumper cordump(true);


int _tmain(int argc, _TCHAR* argv[])
{
	using namespace g3;

	std::string strFileName(argv[0]);
	strFileName = strFileName.substr(strFileName.rfind("\\"));
	std::cout <<"app name:"<< strFileName.c_str()<<std::endl;

	//std::unique_ptr<LogWorker> logworker{ LogWorker::createLogWorker() };

	//std::cout << "createLogWorker" << std::endl;

	//auto sinkHandle = logworker->addSink(std2::make_unique<MyCustFileSink>(strFileName, path_to_log_file),
	//	&MyCustFileSink::fileWrite);

	//std::cout << "begin to initializeLogging" << std::endl;

	//initializeLogging(logworker.get());

	//CMiniDumper cordump(true);

	//std::future<std::string> log_file_name = sinkHandle->call(&MyCustFileSink::fileName);
	////sinkHandle->call(&g3::FileSink::overrideLogDetails, &LogMessage::FullLogDetailsToString);

	//std::cout << "**** G3LOG FATAL EXAMPLE ***\n\n"
	//	<< "Choose your type of fatal exit, then "
	//	<< " read the generated log and backtrace.\n"
	//	<< "The logfile is generated at:  [" << log_file_name.get() << "]\n\n" << std::endl;


	//LOGF(G3LOG_DEBUG, "Fatal exit example starts now, it's as easy as  %d", 123);

	//std::cout << g3::localtime_formatted(std::chrono::system_clock::now(), "%a %b %d %H:%M:%S %Y") << std::endl;
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::cout << g3::localtime_formatted(std::chrono::system_clock::now(), "%%Y/%%m/%%d %%H:%%M:%%S = %Y/%m/%d %H:%M:%S") << std::endl;
	//std::cout <<"new time : "<< g3::localtime_formatted(std::chrono::system_clock::now(), "%Y-%m-%d %H:%M:%S %f")<<std::endl; // %Y/%m/%d

	//funcTest();
	int iTryTime = 0;
	while (true)
	{
		//LOGF(G3LOG_DEBUG, "Fatal exit example starts now, it's as easy as  %d", 123);
		std::cout << "Fatal exit example starts now," << std::endl;
		Sleep(1000);

		if (iTryTime++ > 10)
		{
			break;
		}
	}

	//InitUnhandledExceptionFilter();

	char* szTemp = (char*)1;
	strcpy_s(szTemp, 1000, "A");

	std::cout<< "Feel free to read the source code also in g3log/example/main_fatal_choice.cpp";

	//LOG(INFO) << "Feel free to read the source code also in g3log/example/main_fatal_choice.cpp";
	return 0;
}

