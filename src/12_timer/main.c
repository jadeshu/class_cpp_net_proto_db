#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#include <Userenv.h>
#pragma comment(lib, "Userenv.lib")

#include "uv.h"

uv_loop_t* loop = NULL;
static uv_timer_t timer;

void on_timer(uv_timer_t* handle)
{
	printf("timer called!!!\n");
	uv_timer_stop(handle);	//停止timer
}
int main(int argc, char* argv[])
{
	loop = uv_default_loop();
	//UV定时器
	uv_timer_init(loop, &timer);

	//启动timer
	//timeout:第一次启动时间隔多长时间
	//repeat ;启动后每隔多少时间调用一次
	uv_timer_start(&timer, on_timer, 1000, 2000);
	//end

	uv_run(loop, UV_RUN_DEFAULT);
	system("pause");
	return 0;
}