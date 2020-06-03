#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <vector>

#pragma comment(lib,"ws2_32.lib")	//仅windows平台可以

using namespace std;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
//消息头
class DataHeader
{
public:
	short dataLength;//数据长度
	short cmd;		//命令
};
//DataPackage
class Login: public DataHeader
{
public:
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
//登入结果，是否登陆成功
class LoginResult : public DataHeader
{
public:
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;	//默认0代表正常
	}
	int result;
};
class Logout : public DataHeader
{
public:
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
//登出结果
class LogoutResult : public DataHeader
{
public:
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};
class NewUserJoin : public DataHeader
{
public:
	NewUserJoin()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

//全局的客户端列表
vector<SOCKET> g_clients;

//处理请求
int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};
	//5 接收客户端的请求,传来的包大于接收长度，后面的留到接下来接收
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "客户端<Socket="<<_cSock<<">已退出，任务结束。" << endl;
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//5 接受客户端请求，包头已经读取，偏移取包体其他信息
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login包含header所以把之前在缓存区的数据一并纳入login
		Login* login = reinterpret_cast<Login*>(szRecv);
		cout << "收到客户端<Socket="<< _cSock
			<< ">请求：CMD_LOGIN, 数据长度：" << header->dataLength
			<< ", userName=" << login->userName
			<< ", PassWord=" << login->PassWord
			<< endl;
		//忽略判断用户密码是否正确的步骤了
		LoginResult ret;
		//7 send 向客户端发送一条数据
		send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		//接受客户端请求
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = reinterpret_cast<Logout*>(szRecv);
		cout << "收到命令：CMD_LOGOUT"
			<< ", 数据长度：" << header->dataLength
			<< ", userName=" << logout->userName
			<< endl;
		//忽略判断用户密码是否正确的步骤了
		LogoutResult ret;
		//7 send 向客户端发送一条数据
		send(_cSock, (char*)& ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
	{
		*header = { 0,CMD_ERROR };
		send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
	}
	break;
	}
}

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


	while (true)
	{
		//socket 集合的结构体
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		//清空集合
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		//将文件描述符_sock加入集合之中
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		//每一次循环将队列中的客户端加入集合
		for (int n = static_cast<int>(g_clients.size() - 1) ; n >=0 ; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		timeval t = { 0,0 };
		//为兼容其他操作系统的 伯克利 socket,Windows下select()的第一个参数无意义;
		//select()第一个参数nfds是一个整数，是指fd_set集合中描述符最大值+1;
		//对应TCP协议中，接收方发送确认号是已发送数据序号的下一个。
		int ret = select(_sock+1, &fdRead, &fdWrite, &fdExp,&t );

		//ret=0无数据
		if (ret < 0)
		{
			cout << "select任务结束" << endl;
			break;
		}
		//检查集合中指定的文件描述符是否存在
		if (FD_ISSET(_sock, &fdRead))
		{
			//将一个给定的文件描述符从集合中删除
			FD_CLR(_sock, &fdRead);
			//4 accept 等待接收客户端链接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;

			//客户端_cSock,服务端_sock
			_cSock = accept(_sock, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{
				cout << "错误，接收到无效客户端SOCKET。" << endl;
			}
			else
			{
				for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n], reinterpret_cast<const char*>(&userJoin), sizeof(NewUserJoin), 0);
				}
				//新客户端加入放入队列里
				g_clients.push_back(_cSock);
				//inet_ntoa()功能是将网络地址转换成“.”点隔的字符串格式
				cout << "新客户端加入：socket = " << _cSock << ",IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
			}
			
		}

		//if(nLen>=sizeof(DataHeader)){}
		
		for (size_t n = 0; n < fdRead.fd_count ; n++)
		{
			//6 处理请求
			if(-1==processor(fdRead.fd_array[n]))
			{
				//处理成功后队列里移除此客户端
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		cout << "空闲时间处理其他业务.." << endl;
	}
	//清理已链接的客户端套接字
	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
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
