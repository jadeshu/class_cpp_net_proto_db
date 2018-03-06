#include <stdio.h>

#include <windows.h>

static int g_int = 100;	//全局
CRITICAL_SECTION cs;	//锁	--线程同步锁
HANDLE hEvent = NULL;

DWORD WINAPI th_fun(LPVOID lpThreadParameter) 
{
	
	while (1)
	{		
		printf("sub thread work!!\n");
		EnterCriticalSection(&cs);
		g_int = 200;
		//printf("sub thread g_int = %d\n", g_int);
		SetEvent(hEvent);
		LeaveCriticalSection(&cs);
		Sleep(1000);	
		
	}
	return 0;
}
int main(int argc, char* argv[]) 
{
	InitializeCriticalSection(&cs);
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	HANDLE hTh = CreateThread(NULL, 0, th_fun, NULL, 0, NULL);

	while (1)
	{		
		//等待事件对象
		WaitForSingleObject(hEvent, INFINITE);	
		printf("main thread work!!\n");
		EnterCriticalSection(&cs);
		g_int = 100;
		//printf("main thread g_int = %d\n", g_int);
		LeaveCriticalSection(&cs);
		Sleep(1000);
	}
	return 0;
}