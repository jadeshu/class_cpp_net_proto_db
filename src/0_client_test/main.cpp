#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

//字符串缓冲区大小
#define MAX_TEXT_LEN 100


int main()
{
	WSADATA wsData;
	int nRet = WSAStartup(0x0202, &wsData);
	if (nRet)
	{
		printf("----WSAStartup初始化失败\n----");
	}

	//创建套接字(创建就知道了是什么协议的套接字)
	SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		return -1;
	}
	printf("----创建套接字成功!----\n");
	struct sockaddr_in serAddr;
	int nSerAddrLen = sizeof(sockaddr_in);	//保存客户端信息字节数
	memset(&serAddr, 0, nSerAddrLen);		//填充数据为0

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(6060);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//套接字与服务器绑定(即系统知道端口 IP)
	if (connect(sClient, (sockaddr*)&serAddr, nSerAddrLen))	//返回0时成功 否则失败或者错误
	{
		DWORD i = GetLastError();
		printf("----连接服务器失败----\n");
		closesocket(sClient);
		WSACleanup();
		return -1;
	}
	printf("----连接服务器成功----\n");

	char RecBuffer[MAX_TEXT_LEN] = {0};
	int nRetBufferLen = 0;
	while (true)
	{	
		printf("----请在下面输入您想跟服务器发送的内容 大小%d字节 逐行发送----\n", MAX_TEXT_LEN);
		//获取输入字符
		gets_s(RecBuffer,MAX_TEXT_LEN);
		
		send(sClient, RecBuffer, strlen(RecBuffer)*sizeof(char), 0);
		
		nRetBufferLen = recv(sClient, RecBuffer, MAX_TEXT_LEN-2, 0);
		int i = GetLastError();
		RecBuffer[nRetBufferLen] = '\0';
		printf("%s\n", RecBuffer);

		//--------------关闭问题待解决--------------------
		//shutdown(sClient,0);
		//closesocket(sClient);
	}
	//shutdown(sClient,0);
	closesocket(sClient);
	WSACleanup();
	return 0;
}