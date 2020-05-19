#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//仅windows平台可以

using namespace std;
struct DataPackage
{
	int age;
	char name[32];
};
int main()
{
//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
//------------------
//1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		cout << "错误，建立socket失败。" << endl;
	}
	else
	{
		cout << "建立socket成功。" << endl;
	}
//2 连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "错误，连接socket失败。";
	}
	else
	{
		cout << "连接socket成功。" << endl;
	}
	
	while (true)
	{
//3 输入请求命令
		char cmdBuf[128] = {};
		cin >> cmdBuf;
//4 处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			cout << "收到退出命令，任务结束。" << endl;
			break;
		}
		else
		{
//5 向服务器发送请求
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}
//6 接收服务器信息 recv
		char recvBuf[128]{};
		int nlen = recv(_sock, recvBuf, 128, 0);
		if (nlen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			cout << "接收到数据：年龄="<<dp->age<<", 姓名="<<dp->name<<endl;
		}
	}

//7 关闭套接字closesocket
	closesocket(_sock);
	//------------------
//清除Windows socket环境
	WSACleanup();
	cout << "已退出。" << endl;
	getchar();
	return 0;
}