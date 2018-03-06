#include <stdio.h>
#include <stdlib.h>

#include <WinSock2.h>
#include <Mswsock.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

enum 
{
	IOCP_ACCPET = 0,
	IOCP_RECV,
	IOCP_WRITE
};

#define MAX_RECV_SIZE 8196
struct io_package
{
	WSAOVERLAPPED overlapped;
	int opt;	//�������� ���� �� д
	//���������ύ����ľ��,accept�ľ��
	int accpet_sock;	
	//����
	WSABUF wsaBuffer;
	//ʵ�ʴ������
	char pkg[MAX_RECV_SIZE];
};

static void post_accept(SOCKET sock)
{
	//����һ��struct io_package�ṹ�ڴ�
	struct io_package* io_data = (struct io_package*)malloc(sizeof(struct io_package));
	memset(io_data, 0, sizeof(struct io_package));

	io_data->wsaBuffer.buf = io_data->pkg;
	io_data->wsaBuffer.len = MAX_RECV_SIZE - 1;
	io_data->opt = IOCP_ACCPET;	//��������

	DWORD dwBytes = 0;
	SOCKET client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	int addr_len = sizeof(struct sockaddr_in) + 16;
	io_data->accpet_sock = client_sock;	//��client socket��ͻ��˽���

	//����һ���첽������������ͻ��˵�����
	AcceptEx(sock, client_sock, io_data->wsaBuffer.buf, 0, addr_len,
		addr_len, &dwBytes, &(io_data->overlapped));
}

static void post_recv(SOCKET sock) {
	//����һ��struct io_package�ṹ�ڴ�
	struct io_package* io_data = (struct io_package*)malloc(sizeof(struct io_package));
	memset(io_data, 0, sizeof(struct io_package));

	io_data->wsaBuffer.buf = io_data->pkg;
	io_data->wsaBuffer.len = MAX_RECV_SIZE - 1;
	io_data->opt = IOCP_RECV;	//��������
	io_data->accpet_sock = sock;

	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	int ret = WSARecv(sock, &(io_data->wsaBuffer), 1, &dwRecv, &dwFlags,
						&(io_data->overlapped), NULL);

}

int main(int argc, char* argv[])
{
	WSADATA wsData;
	WSAStartup(MAKEWORD(2, 2), &wsData);

	//����IOCP
	HANDLE hIO = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIO == INVALID_HANDLE_VALUE)
	{
		goto filed;
	}

	//���� ��  �����׽��� ============================================
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
	{
		goto filed;
		return 0;
	}

	SOCKADDR_IN s_in;
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(6060);
	s_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int err = bind(server, (SOCKADDR*)&s_in, sizeof(s_in));
	if (err == SOCKET_ERROR)
	{
		printf("���׽���ʧ��");
		goto filed;
	}

	err = listen(server, 5);
	if (err == SOCKET_ERROR)
	{
		goto filed;
	}
	//end


	//��IOCP���������ǵ�server
	//���������� ���Լ����������
	CreateIoCompletionPort((HANDLE)server, hIO, (DWORD)0, 0);
	//����һ�������û�����������
	post_accept(server);

	while (1)
	{
		DWORD dwTrans = 0,udata =0;
		struct io_package* io_data = NULL;
		//ͨ����ɶ˿�������������Ľ��
		int ret = GetQueuedCompletionStatus(hIO, &dwTrans, &udata, 
						(LPOVERLAPPED)&io_data,WSA_INFINITE);
		//DWORD e = GetLastError();
		if (ret == 0)	//����
		{
			if (io_data->opt == IOCP_RECV)
			{
				closesocket(io_data->accpet_sock);
				printf("%d �Ѿ�����\n", io_data->accpet_sock);
				free(io_data);
				io_data = NULL;
			}
			else if (io_data->opt == IOCP_ACCPET)
			{
				free(io_data);
				io_data = NULL;
				post_accept(server);
			}
			continue;
		}

		if (dwTrans == 0 && io_data->opt == IOCP_RECV)
		{
			//�ر�socket������
			closesocket(io_data->accpet_sock);
			free(io_data);
			io_data = NULL;
			continue;
			//end
		}

		switch (io_data->opt)
		{
		case IOCP_ACCPET:
		{		
			//int addr_size = (sizeof(struct sockaddr_in) + 16);
				
			//struct sockaddr_in* l_addr = NULL;
			//int l_len = sizeof(struct sockaddr_in);
			//struct sockaddr_in* r_addr = NULL;
			//int r_len = sizeof(struct sockaddr_in);

			//GetAcceptExSockaddrs(io_data->wsaBuffer.buf, 0, addr_size, 
			//		addr_size,&l_addr, &l_len, &r_addr, &r_len);

			int client_sock = io_data->accpet_sock;
			//���½�����clientҲ������ɶ˿ڣ���ϵͳ��æ����
			CreateIoCompletionPort((HANDLE)client_sock, hIO, (DWORD)client_sock, 0);
			//Ͷ��һ����������
			post_recv(client_sock);

			//����Ͷ��һ����������
			free(io_data);
			io_data = NULL;
			post_accept(server);
			//end
		}
		break;
		case IOCP_RECV:
		{
			//dwTrans �������ݵĴ�С
			//socket io_data->accpet_sock
			//buf io_data->wsaBuffer.buf 
			io_data->pkg[dwTrans] = 0;
			printf("IOCP recv %d--->%s\n",io_data->accpet_sock, io_data->pkg);

			//����Ͷ��һ������
			DWORD dwRecv = 0;
			DWORD dwFlags = 0;
			int ret = WSARecv(io_data->accpet_sock, &(io_data->wsaBuffer), 1, 
								&dwRecv, &dwFlags,&(io_data->overlapped), NULL);
			//end
		}
		break;

		//default:
		//	break;
		}
	}

filed:
	if (hIO != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hIO);
	}
	WSACleanup();
	return 0;
}