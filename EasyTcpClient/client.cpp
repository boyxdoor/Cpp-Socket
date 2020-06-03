#define WIN32_LEAN_AND_MEAN	//尽量避免头文件中库中的冲突
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

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
}

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
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		int ret = select(_sock, &fdReads, 0, 0, &t);
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

		cout << "空闲时间处理其他业务.." << endl;

		Login login;
		strcpy(login.userName, "ljd");
		strcpy(login.PassWord, "ljd");
		send(_sock, reinterpret_cast<const char*>(&login),
			sizeof(Login), 0);
		//Sleep(1000);
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