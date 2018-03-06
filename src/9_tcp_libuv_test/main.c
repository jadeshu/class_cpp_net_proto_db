#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")
#include "../../3rd/libuv/include/uv.h"
//代码查看 uv_tcp_t > uv_stream_t > uv_handle_t
//struct uv_tcp_s
//{
//	UV_HANDLE_FIELDS
//	UV_STREAM_FIELDS
//	UV_TCP_PRIVATE_FIELDS
//};
//
//struct uv_stream_s
//{
//	UV_HANDLE_FIELDS
//	UV_STREAM_FIELDS
//};
//
//struct uv_handle_s {
//	UV_HANDLE_FIELDS
//};

uv_loop_t* loop = NULL;
void on_close(uv_handle_t* handle)
{
	//读取意外关闭了释放申请的内存
	if (handle->data != NULL)
	{
		free(handle->data);
		handle->data = NULL;
	}
	//end
}
void on_shutdown(uv_shutdown_t* req, int status)
{
	printf("已经退出!!!\n");
	uv_close((uv_handle_t*)(req->handle), on_close);

	//释放申请的内存
	free(req);
	//end
}

void on_after_write(uv_write_t* req, int status)
{
	if (status == 0)
	{
		printf("write OK\n");
	}

	//释放申请的内存
	uv_buf_t* w_buf = req->data;
	if (w_buf != NULL)
	{
		free(w_buf);
	}
	free(req);
	//end
}
//参数：
//uv_stream_t* stream --->uv_tcp_t;
//nread:读到了多少字节的数据
//buf:数据读取到那个buf？？  buf->base
void on_after_read(uv_stream_t* stream,ssize_t nread,const uv_buf_t* buf)
{
	//读取数据
	if (nread < 0)
	{
		uv_shutdown_t* req = malloc(sizeof(uv_shutdown_t));
		memset(req, 0, sizeof(uv_shutdown_t));
		uv_shutdown(req, stream, on_shutdown);
		return;
	}
	buf->base[nread] = 0;
	printf("recv %d--->%s\n", nread, buf->base);
	//end

	//发送数据
	uv_write_t* w_req = malloc(sizeof(uv_write_t));
	memset(w_req, 0, sizeof(uv_write_t));

	uv_buf_t* w_buf = malloc(sizeof(uv_buf_t));
	w_req->data = w_buf;		//保存以便后面释放内存
	w_buf->base = buf->base;
	w_buf->len = nread;
	
	uv_write(w_req, stream, w_buf, 1, on_after_write);
	//end
}

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	if (handle->data != NULL)
	{
		free(handle->data);
		handle->data = NULL;
	}

	buf->base = malloc(suggested_size + 1);
	memset(buf->base, 0, suggested_size + 1);
	buf->len = suggested_size;
	handle->data = buf->base;	//保存 以便后面释放内存
}

void on_new_connection(uv_stream_t* server, int status)
{
	if (status < 0) {
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		// error!
		return;
	}

	uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(client, 0, sizeof(uv_tcp_t));

	uv_tcp_init(loop, client);
	if (uv_accept(server, (uv_stream_t*)client) == 0) {
		uv_read_start((uv_stream_t*)client, alloc_buffer, on_after_read);
	}
}

//uv_tcp_t > uv_stream_t > uv_handle_t
int main(int argc, char* argv[])
{
	loop = uv_default_loop();
	uv_tcp_t server;
	uv_tcp_init(loop, &server);

	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 6060, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*)&server, 5, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}

	uv_run(loop, UV_RUN_DEFAULT);

	system("pause");
	return 0;
}