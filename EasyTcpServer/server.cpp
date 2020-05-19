#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//仅windows平台可以

using namespace std;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
//消息头
struct DataHeader
{
	short dataLength;//数据长度
	short cmd;		//命令
};
//DataPackage
struct Login
{
	char userName[32];
	char PassWord[32];
};
//登入结果，是否登陆成功
struct LoginResult
{
	int result;
};
struct Logout
{
	char userName[32];
};
//登出结果
struct LogoutResult
{
	int result;
};

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

	//客户端_cSock,服务端_sock
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
		DataHeader header{};
		//5 接收客户端的请求
		int nLen = recv(_cSock, (char*)& header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			cout << "客户端已退出，任务结束。" << endl;
			break;
		}
		cout << "收到命令：" << header.cmd<<", 数据长度："
			<<header.dataLength<<endl;
		//6 处理请求
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login{};
			//接受客户端请求
			recv(_cSock, (char*)&login, sizeof(Login), 0);
			//忽略判断用户密码是否正确的步骤了
			LoginResult ret{0};
			//返回消息头
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			//返回登录结果
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout logout{};
			//接受客户端请求
			recv(_cSock, (char*)&logout, sizeof(Logout), 0);
			//忽略判断用户密码是否正确的步骤了
			LogoutResult ret{ 1 };
			////7 send 向客户端发送一条数据 返回消息头
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			//返回登录结果
			send(_cSock, (char*)& ret, sizeof(LogoutResult), 0);
		}
		break;
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			break;
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
