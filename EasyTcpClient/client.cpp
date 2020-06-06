#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include <thread>

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
class Login : public DataHeader
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

int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};
	//5 接收客户端的请求,传来的包大于接收长度，后面的留到接下来接收
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "与服务器断开连接，任务结束。" << endl;
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{

		//5 接受客户端请求，包头已经读取，偏移取包体其他信息
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login包含header所以把之前在缓存区的数据一并纳入login
		LoginResult* login = reinterpret_cast<LoginResult*>(szRecv);
		cout << "收到服务器消息：CMD_LOGIN_RESULT, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		//5 接受客户端请求，包头已经读取，偏移取包体其他信息
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login包含header所以把之前在缓存区的数据一并纳入login
		LogoutResult* logout = reinterpret_cast<LogoutResult*>(szRecv);
		cout << "收到服务器消息：CMD_LOGOUT_RESULT, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		//5 接受客户端请求，包头已经读取，偏移取包体其他信息
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login包含header所以把之前在缓存区的数据一并纳入login
		NewUserJoin* userJoin = reinterpret_cast<NewUserJoin*>(szRecv);
		cout << "收到服务器消息：CMD_NEW_USER_JOIN, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	default:
	{
		*header = { 0,CMD_ERROR };
		send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
	}
	break;
	}
	return 0;
}

//线程运行标志
bool g_bRun = true;

//线程函数接受并处理用户输入的命令
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			cout << "退出cmdThread线程" << endl;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "ljd");
			strcpy(login.PassWord, "ljd");
			send(sock, reinterpret_cast<const char*>(&login),
				sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "ljd");
			send(sock, reinterpret_cast<const char*>(&logout),
				sizeof(Logout), 0);
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}

}

int main()
{
#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
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
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//Windows连接本机地址
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.74.1");//linux系统下连接Windows的IP地址
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "错误，连接socket失败。";
	}
	else
	{
		cout << "连接socket成功。" << endl;
	}

	//启动线程
	thread t1(cmdThread, _sock);
	t1.detach();

	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		int ret = select(_sock+1, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			cout << "select任务结束1" << endl;
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				cout << "select任务结束2" << endl;
				break;
			}
		}
		//线程thread

		//cout << "空闲时间处理其他业务.." << endl;

	}
#ifdef _WIN32
	//7 关闭套接字closesocket
	closesocket(_sock);
	//------------------
	//清除Windows socket环境
	WSACleanup();
#else
	//linux关闭
	close(_sock);
#endif
	cout << "已退出。" << endl;
	getchar();
	return 0;
}