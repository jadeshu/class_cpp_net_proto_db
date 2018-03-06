#include <stdio.h>

#include <windows.h>
int main(int argc, char* argv[])
{
	//异步读取文件

	//打开文件   FILE_FLAG_OVERLAPPED标志异步
	HANDLE hf = CreateFile(L"in.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (hf == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	char buf[1024] = { 0 };
	OVERLAPPED ov = { 0 };
	ov.hEvent = hEvent;
	ov.Offset = 1;		//文件从哪开始读取	低于32位时
	ov.OffsetHigh = 0;	//文件读取高位  高于32位(4G大小)时

	int nRead = 0;
	ReadFile(hf, buf, 1024, &nRead, &ov);
	if (GetLastError() == ERROR_IO_PENDING)
	{
		WaitForSingleObject(hEvent, INFINITE);
		nRead = ov.InternalHigh;
		buf[nRead] = 0;
		printf("%s", buf);
	}
	getchar();
	CloseHandle(hEvent);
	CloseHandle(hf);
	return 0;
}