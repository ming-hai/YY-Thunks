﻿#include "pch.h"
#include "src/Thunks/api-ms-win-core-threadpool.hpp"
#include <process.h>

namespace api_ms_win_core_threadpool
{
	TEST_CLASS(SubmitThreadpoolWork)
	{
	public:
		SubmitThreadpoolWork()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_SetEventWhenCallbackReturns = true;
			YY::Thunks::aways_null_try_get_ReleaseSemaphoreWhenCallbackReturns = true;
		}

		TEST_METHOD(一般行为验证)
		{
			long RunCount = 0;


			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(Work);

			::SubmitThreadpoolWork(Work);
			Sleep(500);
			Assert::AreEqual(RunCount, 1l);

			::SubmitThreadpoolWork(Work);
			Sleep(500);
			Assert::AreEqual(RunCount, 2l);
			
			::SubmitThreadpoolWork(Work);
			::SubmitThreadpoolWork(Work);
			Sleep(500);
			Assert::AreEqual(RunCount, 4l);
			
			Sleep(500);
			Assert::AreEqual(RunCount, 4l);

			CloseThreadpoolWork(Work);
		}
	};

	TEST_CLASS(WaitForThreadpoolWorkCallbacks)
	{
	public:
		WaitForThreadpoolWorkCallbacks()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_WaitForThreadpoolWorkCallbacks = true;
		}

		TEST_METHOD(一般行为验证)
		{
			long RunCount = 0;

			auto Work = ::CreateThreadpoolWork([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);

					Sleep(1 * 1000);

					InterlockedIncrement((long*)Context);

				}, & RunCount, nullptr);

			Assert::IsNotNull(Work);

			::SubmitThreadpoolWork(Work);

			auto h = (HANDLE)_beginthreadex(nullptr, 0, [](void* Work) -> unsigned
				{
					::WaitForThreadpoolWorkCallbacks((PTP_WORK)Work, FALSE);

					return 0;
				}, Work, 0, nullptr);

			Assert::IsNotNull(h);

			Assert::AreEqual(WaitForSingleObject(h, 2 * 1000), WAIT_OBJECT_0, L"2秒内必须完成，这是预期。");

			CloseHandle(h);

			Assert::AreEqual(RunCount, 1l);

			::SubmitThreadpoolWork(Work);
			::SubmitThreadpoolWork(Work);
			::SubmitThreadpoolWork(Work);

			h = (HANDLE)_beginthreadex(nullptr, 0, [](void* Work) -> unsigned
				{
					::WaitForThreadpoolWorkCallbacks((PTP_WORK)Work, FALSE);

					return 0;
				}, Work, 0, nullptr);

			Assert::IsNotNull(h);

			Assert::AreEqual(WaitForSingleObject(h, 6 * 1000), WAIT_OBJECT_0, L"6秒内必须完成，这是预期。");

			CloseHandle(h);

			Assert::AreEqual(RunCount, 4l);


			CloseThreadpoolWork(Work);
		}

	};


	TEST_CLASS(SetThreadpoolTimer)
	{
	public:
		SetThreadpoolTimer()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;
		}

		TEST_METHOD(当DueTime数值为0时)
		{
			//数值为 0 时则立即触发定时器
			long RunCount = 0;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(pTimer);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 800, 0);
			Sleep(100);
			Assert::AreEqual(RunCount, 1l);

			Sleep(900);

			Assert::AreEqual(RunCount, 2l);

			::CloseThreadpoolTimer(pTimer);
		}

		TEST_METHOD(当DueTime是绝对时间)
		{
			long RunCount = 0;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(pTimer);

			CFileTime ftDueTime = {};
			GetSystemTimeAsFileTime(&ftDueTime);

			//2秒后触发此任务
			ftDueTime += CFileTime::Second * 2;
			
			::SetThreadpoolTimer(pTimer, &ftDueTime, 400, 0);
			Sleep(100);
			Assert::AreEqual(RunCount, 0l);

			Sleep(2 * 1000);

			Assert::AreEqual(RunCount, 1l);

			Sleep(900);
			Assert::AreEqual(RunCount, 3l);

			::CloseThreadpoolTimer(pTimer);
		}


		TEST_METHOD(当DueTime是相对时间)
		{
			long RunCount = 0;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(pTimer);

			//2秒后执行此任务
			CFileTime ftDueTime = CFileTime::Second * 2 * -1;

			::SetThreadpoolTimer(pTimer, &ftDueTime, 400, 0);
			Sleep(100);
			Assert::AreEqual(RunCount, 0l);

			Sleep(2 * 1000);

			Assert::AreEqual(RunCount, 1l);

			Sleep(900);
			Assert::AreEqual(RunCount, 3l);

			::CloseThreadpoolTimer(pTimer);
		}

		TEST_METHOD(当DueTime是nullptr)
		{
			long RunCount = 0;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(pTimer);
			CFileTime ftDueTime{};

			::SetThreadpoolTimer(pTimer, &ftDueTime, 400, 0);

			Sleep(900);
			Assert::AreEqual(RunCount, 3l);

			::SetThreadpoolTimer(pTimer, nullptr, 0, 0);

			Sleep(900);
			Assert::AreEqual(RunCount, 3l);

			::CloseThreadpoolTimer(pTimer);
		}

		TEST_METHOD(一次性定时器)
		{
			long RunCount = 0;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);

					InterlockedIncrement((long*)Context);

				}, &RunCount, nullptr);

			Assert::IsNotNull(pTimer);
			CFileTime ftDueTime{};

			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);

			Sleep(500);
			Assert::AreEqual(RunCount, 1l);

			Sleep(500);
			Assert::AreEqual(RunCount, 1l);

			Sleep(500);
			Assert::AreEqual(RunCount, 1l);

			::CloseThreadpoolTimer(pTimer);
		}
	};


	TEST_CLASS(SetEventWhenCallbackReturns)
	{
	public:
		SetEventWhenCallbackReturns()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;

			YY::Thunks::aways_null_try_get_SetEventWhenCallbackReturns = true;
			//YY::Thunks::aways_null_try_get_ReleaseSemaphoreWhenCallbackReturns = true;
		}

		struct UserData
		{
			HANDLE hHandle;
			long RunCount;

			UserData()
				: hHandle(CreateEventW(nullptr, FALSE, FALSE, nullptr))
				, RunCount(0)
			{
				Assert::IsNotNull(hHandle);
			}

			~UserData()
			{
				if (hHandle)
					CloseHandle(hHandle);
			}
		};
		TEST_METHOD(任务测试)
		{
			UserData Data;

			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::SetEventWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);
				}, &Data, nullptr);

			Assert::IsNotNull(Work);
			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 0), (DWORD)WAIT_TIMEOUT);

			::SubmitThreadpoolWork(Work);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			CloseThreadpoolWork(Work);
		}

		TEST_METHOD(定时器测试)
		{
			UserData Data;

			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::SetEventWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);
				}, &Data, nullptr);

			Assert::IsNotNull(pTimer);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 0), (DWORD)WAIT_TIMEOUT);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			::CloseThreadpoolTimer(pTimer);
		}
	};

	TEST_CLASS(ReleaseSemaphoreWhenCallbackReturns)
	{
	public:
		ReleaseSemaphoreWhenCallbackReturns()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;

			YY::Thunks::aways_null_try_get_ReleaseSemaphoreWhenCallbackReturns = true;
		}

		struct UserData
		{
			HANDLE hHandle;
			long RunCount;

			UserData()
				: hHandle(CreateSemaphoreW(nullptr, 0, 1, nullptr))
				, RunCount(0)
			{
				Assert::IsNotNull(hHandle);
			}

			~UserData()
			{
				if(hHandle)
					CloseHandle(hHandle);
			}
		};


		TEST_METHOD(任务测试)
		{
			UserData Data;

			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::ReleaseSemaphoreWhenCallbackReturns(Instance, Data.hHandle, 1);
					InterlockedIncrement(&Data.RunCount);
				}, &Data, nullptr);

			Assert::IsNotNull(Work);
			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 0), (DWORD)WAIT_TIMEOUT);

			::SubmitThreadpoolWork(Work);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			CloseThreadpoolWork(Work);

		}

		TEST_METHOD(定时器测试)
		{
			UserData Data;

			Assert::IsNotNull(Data.hHandle);

			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::ReleaseSemaphoreWhenCallbackReturns(Instance, Data.hHandle, 1);
					InterlockedIncrement(&Data.RunCount);

				}, &Data, nullptr);

			Assert::IsNotNull(pTimer);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 0), (DWORD)WAIT_TIMEOUT);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			::CloseThreadpoolTimer(pTimer);
		}
	};


	TEST_CLASS(ReleaseMutexWhenCallbackReturns)
	{
	public:
		ReleaseMutexWhenCallbackReturns()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;

			YY::Thunks::aways_null_try_get_ReleaseMutexWhenCallbackReturns = true;
		}

		struct UserData
		{
			HANDLE hHandle;
			long RunCount;

			UserData()
				: hHandle(CreateMutexW(nullptr, FALSE, nullptr))
				, RunCount(0)
			{
				Assert::IsNotNull(hHandle);
			}

			~UserData()
			{
				if (hHandle)
					CloseHandle(hHandle);
			}
		};

		TEST_METHOD(任务测试)
		{
			UserData Data;

			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					auto dwWaitResult = WaitForSingleObject(Data.hHandle, 1000);
					Assert::AreEqual(dwWaitResult, WAIT_OBJECT_0);
					::ReleaseMutexWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);

				}, & Data, nullptr);

			Assert::IsNotNull(Work);

			::SubmitThreadpoolWork(Work);
			Sleep(100);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			CloseThreadpoolWork(Work);

		}

		TEST_METHOD(定时器测试)
		{
			UserData Data;			

			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					auto dwWaitResult = WaitForSingleObject(Data.hHandle, 1000);
					Assert::AreEqual(dwWaitResult, WAIT_OBJECT_0);
					::ReleaseMutexWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);
				}, &Data, nullptr);

			Assert::IsNotNull(pTimer);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);
			Sleep(100);

			Assert::AreEqual(WaitForSingleObject(Data.hHandle, 5 * 1000), WAIT_OBJECT_0, L"5秒内句柄必须有信号");
			Assert::AreEqual(Data.RunCount, 1l);

			::CloseThreadpoolTimer(pTimer);
		}
	};


	TEST_CLASS(LeaveCriticalSectionWhenCallbackReturns)
	{
	public:
		LeaveCriticalSectionWhenCallbackReturns()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;

			YY::Thunks::aways_null_try_get_LeaveCriticalSectionWhenCallbackReturns = true;
		}

		struct UserData
		{
			BOOL bInit;
			CRITICAL_SECTION cs;
			long RunCount;

			UserData()
				: bInit(false)
				, RunCount(0)
			{
				bInit = InitializeCriticalSectionAndSpinCount(&cs, 4000);

				Assert::IsTrue(bInit);
			}

			~UserData()
			{
				if (bInit)
				{
					DeleteCriticalSection(&cs);
					bInit = FALSE;
				}
			}
		};

		TEST_METHOD(任务测试)
		{
			UserData Data;

			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;

					EnterCriticalSection(&Data.cs);
					::LeaveCriticalSectionWhenCallbackReturns(Instance, &Data.cs);
					InterlockedIncrement(&Data.RunCount);

				}, &Data, nullptr);

			Assert::IsNotNull(Work);

			::SubmitThreadpoolWork(Work);
			Sleep(100);

			int i = 0;
			for (; i != 100; ++i)
			{
				if (TryEnterCriticalSection(&Data.cs))
				{
					break;
				}

				Sleep(50);
			}

			Assert::AreNotEqual(i, 100, L"100次尝试后必须成功");
			Assert::AreEqual(Data.RunCount, 1l);

			LeaveCriticalSection(&Data.cs);

			CloseThreadpoolWork(Work);

		}

		TEST_METHOD(定时器测试)
		{
			UserData Data;


			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;

					EnterCriticalSection(&Data.cs);
					::LeaveCriticalSectionWhenCallbackReturns(Instance, &Data.cs);
					InterlockedIncrement(&Data.RunCount);

				}, &Data, nullptr);

			Assert::IsNotNull(pTimer);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);
			Sleep(100);

			int i = 0;
			for (; i != 100; ++i)
			{
				if (TryEnterCriticalSection(&Data.cs))
				{
					break;
				}

				Sleep(50);
			}

			Assert::AreNotEqual(i, 100, L"100次尝试后必须成功");
			Assert::AreEqual(Data.RunCount, 1l);

			LeaveCriticalSection(&Data.cs);
			::CloseThreadpoolTimer(pTimer);
		}
	};


	TEST_CLASS(FreeLibraryWhenCallbackReturns)
	{
	public:
		FreeLibraryWhenCallbackReturns()
		{
			YY::Thunks::aways_null_try_get_CreateThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolWork = true;
			YY::Thunks::aways_null_try_get_SubmitThreadpoolWork = true;

			YY::Thunks::aways_null_try_get_CreateThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_CloseThreadpoolTimer = true;
			YY::Thunks::aways_null_try_get_SetThreadpoolTimer = true;

			YY::Thunks::aways_null_try_get_FreeLibraryWhenCallbackReturns = true;
		}

		struct UserData
		{
			HMODULE hHandle;
			long RunCount;

			UserData(HMODULE hMod)
				: hHandle(hMod)
				, RunCount(0)
			{
				Assert::IsNotNull(hHandle);
			}

			~UserData()
			{
				if (hHandle)
					CloseHandle(hHandle);
			}
		};

		TEST_METHOD(任务测试)
		{
			UserData Data(LoadLibraryExW(L"regedit.exe", nullptr, LOAD_LIBRARY_AS_DATAFILE));

			auto Work = ::CreateThreadpoolWork([](_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_WORK              Work)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::FreeLibraryWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);

				}, &Data, nullptr);

			Assert::IsNotNull(Work);

			::SubmitThreadpoolWork(Work);
			Sleep(500);

			Assert::IsFalse(FreeLibrary(Data.hHandle));
			Assert::AreEqual(Data.RunCount, 1l);

			CloseThreadpoolWork(Work);

		}

		TEST_METHOD(定时器测试)
		{
			UserData Data(LoadLibraryExW(L"splwow64.exe", nullptr, LOAD_LIBRARY_AS_DATAFILE));;

			auto pTimer = ::CreateThreadpoolTimer([](
				_Inout_     PTP_CALLBACK_INSTANCE Instance,
				_Inout_opt_ PVOID                 Context,
				_Inout_     PTP_TIMER             Timer)
				{
					Assert::IsNotNull(Context);
					auto& Data = *(UserData*)Context;
					::FreeLibraryWhenCallbackReturns(Instance, Data.hHandle);
					InterlockedIncrement(&Data.RunCount);
				}, &Data, nullptr);

			Assert::IsNotNull(pTimer);

			FILETIME ftDueTime = {};
			::SetThreadpoolTimer(pTimer, &ftDueTime, 0, 0);
			Sleep(500);

			Assert::IsFalse(FreeLibrary(Data.hHandle));
			Assert::AreEqual(Data.RunCount, 1l);
			//FreeLibrary()
			::CloseThreadpoolTimer(pTimer);
		}
	};
}