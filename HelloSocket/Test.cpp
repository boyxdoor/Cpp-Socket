#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#include <WinSock2.h>
#include <Windows.h>

//#pragma comment(lib,"ws2_32.lib")	//仅windows平台可以

int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------------

	//------------------
	//清除Windows socket环境
	WSACleanup();
	return 0;
}