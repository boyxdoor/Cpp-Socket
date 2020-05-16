#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//仅windows平台可以

using namespace std;

int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------------
	//用socket API建立简易TCP服务器
	//1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2 bind 绑定用于接收客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); //主机向网络转换host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");绑定IP地址
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		cout << "错误，绑定网络端口失败。\n";
	}
	else
	{
		cout << "绑定网络端口成功。\n";
	}
	//3 listen 监听网络端口
	if(SOCKET_ERROR==listen(_sock, 5))
	{
		cout << "错误，监听网络端口失败。\n";
	}
	else
	{
		cout << "监听网络端口成功。\n";
	}
	//4 accept 等待接收客户端链接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;


	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		cout << "错误，接收到无效客户端SOCKET。"<<endl ;
	}
	//inet_ntoa()功能是将网络地址转换成“.”点隔的字符串格式
	cout << "新客户端加入：socket = " << _cSock<<",IP = "<<inet_ntoa(clientAddr.sin_addr) << endl;
	char _recvBuf[128] = {};

	while (true)
	{
		//5 接收客户端的请求
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			cout << "客户端已退出，任务结束。" << endl;
			break;
		}
		cout << "收到命令：" << _recvBuf<<endl;
		//6 处理请求
		if (0 == strcmp(_recvBuf, "getName"))
		{
			//7 send 向客户端发送一条数据
			const char msgBuf[] = "Xiao Liang";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//结尾符一并发过去
		}
		else if (0 == strcmp(_recvBuf, "getAge"))
		{
			//7 send 向客户端发送一条数据
			const char msgBuf[] = "80";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//结尾符一并发过去
		}
		else
		{
			//7 send 向客户端发送一条数据
			const char msgBuf[] = "???";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//结尾符一并发过去
		}

	}
	
	//8 关闭套接字closesocket
	closesocket(_sock);
	//------------------
	//清除Windows socket环境
	WSACleanup();
	cout << "已退出。" << endl;
	getchar();
	return 0;
}
