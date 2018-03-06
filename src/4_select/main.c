#include <stdio.h>

#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

static int g_count = 0;
static SOCKET g_sockets[64] = {0};
int main(int argc, char* argv[])
{
	WSADATA wsData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (err != 0) 
	{
		printf("WSAStartup failed with error: %d\n", err);
		return 0;
	}

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
	{
		printf("socket套接字失败!!");
		WSACleanup();
		return 0;
	}

	SOCKADDR_IN s_in;
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(6060);
	s_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	err = bind(server, (SOCKADDR*)&s_in, sizeof(s_in));
	if (err == SOCKET_ERROR)
	{
		printf("bind套接字失败!!");
		WSACleanup();
	}

	err = listen(server, 5);
	if (err == SOCKET_ERROR)
	{
		printf("listen套接字失败!!");
		WSACleanup();
	}

	fd_set fs_read;
	g_sockets[g_count++] = server;
	
	while (1)
	{
		FD_ZERO(&fs_read);

		for (int i = 0; i < g_count;i++)
		{
			FD_SET(g_sockets[i], &fs_read);
		}

		int nfs_count = select(0, &fs_read, NULL, NULL, NULL);
		if (nfs_count < 0) {
			printf("select error\n");
			continue;
		}
		else if (nfs_count == 0) {
			printf("select time out\n");
			continue;
		}
		else {
			if (FD_ISSET(server, &fs_read)) { // 服务器有人发送连接请求了
				struct sockaddr_in c_address;
				int address_len = sizeof(c_address);
				int client = accept(server, (struct sockaddr*)&c_address, &address_len);
				if (client > 0) {
					printf("new client comming...! %s：%d\n", inet_ntoa(c_address.sin_addr), ntohs(c_address.sin_port));
					g_sockets[g_count++] = client;
				}
				else {
					printf("error\n");
				}
			}
			//else {
			//	foreach_session(fd_isset_in_sessions, (void*)&socket_set);
			//}
		}				
	}

	WSACleanup();
	return 0;
}