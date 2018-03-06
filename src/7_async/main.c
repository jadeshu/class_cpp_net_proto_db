#include <stdio.h>

#include <windows.h>
int main(int argc, char* argv[])
{
	//�첽��ȡ�ļ�

	//���ļ�   FILE_FLAG_OVERLAPPED��־�첽
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
	ov.Offset = 1;		//�ļ����Ŀ�ʼ��ȡ	����32λʱ
	ov.OffsetHigh = 0;	//�ļ���ȡ��λ  ����32λ(4G��С)ʱ

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