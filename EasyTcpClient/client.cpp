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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			//5 向服务器发送请求
			Login login;
			strcpy(login.userName, "ljd");
			strcpy(login.PassWord, "ljdmm");
			//包体包含了包头
			send(_sock, (const char*)& login, sizeof(Login), 0);
//6 接收服务器信息 recv
			LoginResult loginRet{};
			recv(_sock, (char*)& loginRet, sizeof(LoginResult), 0);
			cout << "LoginResult:" << loginRet.result << endl;
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "ljd");
//5 向服务器发送请求
			send(_sock, (const char*)& logout, sizeof(Logout), 0);//包体
//6 接收服务器信息 recv
			LogoutResult logoutRet{};
			recv(_sock, (char*)& logoutRet, sizeof(LogoutResult), 0);
			cout << "LogoutResult:" << logoutRet.result << endl;
		}
		else
		{
			cout << "不支持的命令，请重新输入。" << endl;
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