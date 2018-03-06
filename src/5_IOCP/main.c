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
	int opt;	//操作类型 监听 读 写
	//就是我们提交请求的句柄,accept的句柄
	int accpet_sock;	
	//缓冲
	WSABUF wsaBuffer;
	//实际存放数据
	char pkg[MAX_RECV_SIZE];
};

static void post_accept(SOCKET sock)
{
	//分配一个struct io_package结构内存
	struct io_package* io_data = (struct io_package*)malloc(sizeof(struct io_package));
	memset(io_data, 0, sizeof(struct io_package));

	io_data->wsaBuffer.buf = io_data->pkg;
	io_data->wsaBuffer.len = MAX_RECV_SIZE - 1;
	io_data->opt = IOCP_ACCPET;	//请求类型

	DWORD dwBytes = 0;
	SOCKET client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	int addr_len = sizeof(struct sockaddr_in) + 16;
	io_data->accpet_sock = client_sock;	//用client socket与客户端交互

	//发送一个异步的请求，来接入客户端的连接
	AcceptEx(sock, client_sock, io_data->wsaBuffer.buf, 0, addr_len,
		addr_len, &dwBytes, &(io_data->overlapped));
}

static void post_recv(SOCKET sock) {
	//分配一个struct io_package结构内存
	struct io_package* io_data = (struct io_package*)malloc(sizeof(struct io_package));
	memset(io_data, 0, sizeof(struct io_package));

	io_data->wsaBuffer.buf = io_data->pkg;
	io_data->wsaBuffer.len = MAX_RECV_SIZE - 1;
	io_data->opt = IOCP_RECV;	//请求类型
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

	//创建IOCP
	HANDLE hIO = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIO == INVALID_HANDLE_VALUE)
	{
		goto filed;
	}

	//创建 绑定  监听套接字 ============================================
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
		printf("绑定套接字失败");
		goto filed;
	}

	err = listen(server, 5);
	if (err == SOCKET_ERROR)
	{
		goto filed;
	}
	//end


	//让IOCP来管理我们的server
	//第三个参数 由自己定义的数据
	CreateIoCompletionPort((HANDLE)server, hIO, (DWORD)0, 0);
	//发送一个监听用户进来的请求
	post_accept(server);

	while (1)
	{
		DWORD dwTrans = 0,udata =0;
		struct io_package* io_data = NULL;
		//通过完成端口来获得这个请求的结果
		int ret = GetQueuedCompletionStatus(hIO, &dwTrans, &udata, 
						(LPOVERLAPPED)&io_data,WSA_INFINITE);
		//DWORD e = GetLastError();
		if (ret == 0)	//意外
		{
			if (io_data->opt == IOCP_RECV)
			{
				closesocket(io_data->accpet_sock);
				printf("%d 已经离线\n", io_data->accpet_sock);
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
			//关闭socket发生了
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
			//将新进来的client也加入完成端口，让系统帮忙管理
			CreateIoCompletionPort((HANDLE)client_sock, hIO, (DWORD)client_sock, 0);
			//投递一个读的请求
			post_recv(client_sock);

			//重新投递一个接收请求
			free(io_data);
			io_data = NULL;
			post_accept(server);
			//end
		}
		break;
		case IOCP_RECV:
		{
			//dwTrans 读到数据的大小
			//socket io_data->accpet_sock
			//buf io_data->wsaBuffer.buf 
			io_data->pkg[dwTrans] = 0;
			printf("IOCP recv %d--->%s\n",io_data->accpet_sock, io_data->pkg);

			//再来投递一个请求
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