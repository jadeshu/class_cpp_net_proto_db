#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

//�ַ�����������С
#define MAX_TEXT_LEN 100


int main()
{
	WSADATA wsData;
	int nRet = WSAStartup(0x0202, &wsData);
	if (nRet)
	{
		printf("----WSAStartup��ʼ��ʧ��\n----");
	}

	//�����׽���(������֪������ʲôЭ����׽���)
	SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		return -1;
	}
	printf("----�����׽��ֳɹ�!----\n");
	struct sockaddr_in serAddr;
	int nSerAddrLen = sizeof(sockaddr_in);	//����ͻ�����Ϣ�ֽ���
	memset(&serAddr, 0, nSerAddrLen);		//�������Ϊ0

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(6060);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//�׽������������(��ϵͳ֪���˿� IP)
	if (connect(sClient, (sockaddr*)&serAddr, nSerAddrLen))	//����0ʱ�ɹ� ����ʧ�ܻ��ߴ���
	{
		DWORD i = GetLastError();
		printf("----���ӷ�����ʧ��----\n");
		closesocket(sClient);
		WSACleanup();
		return -1;
	}
	printf("----���ӷ������ɹ�----\n");

	char RecBuffer[MAX_TEXT_LEN] = {0};
	int nRetBufferLen = 0;
	while (true)
	{	
		printf("----��������������������������͵����� ��С%d�ֽ� ���з���----\n", MAX_TEXT_LEN);
		//��ȡ�����ַ�
		gets_s(RecBuffer,MAX_TEXT_LEN);
		
		send(sClient, RecBuffer, strlen(RecBuffer)*sizeof(char), 0);
		
		nRetBufferLen = recv(sClient, RecBuffer, MAX_TEXT_LEN-2, 0);
		int i = GetLastError();
		RecBuffer[nRetBufferLen] = '\0';
		printf("%s\n", RecBuffer);

		//--------------�ر���������--------------------
		//shutdown(sClient,0);
		//closesocket(sClient);
	}
	//shutdown(sClient,0);
	closesocket(sClient);
	WSACleanup();
	return 0;
}