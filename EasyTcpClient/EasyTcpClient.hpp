#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN    //尽量避免头文件中库中的冲突
	#include <WinSock2.h>
	#include <Windows.h>
	#pragma comment(lib,"ws2_32.lib")    //仅windows平台可以
#else
	#include <unistd.h> //uni std
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include "MessageHeader.hpp"
#include <iostream>
#include <thread>

using namespace std;

class EasyTcpClient
{
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();

	//初始化socket
	bool InitSocket();

	//连接服务器
	int Connect(const char* ip, unsigned short port);

	// 关闭套接字closesocket
	void Close();

	//接收数据 处理粘包 拆包,包含OnNetMsg(header)操作
	int RcevData(SOCKET _sock);

	//发送数据,客户端中交给线程处理
	int SendData(DataHeader* header);

	//响应网络请求
	void OnNetMsg(DataHeader* header);

	//查询网络消息,包含RcevData(_sock)操作
	bool OnRun();

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
private:
	SOCKET _sock;
};

EasyTcpClient::EasyTcpClient()
{
	_sock = INVALID_SOCKET;
}

EasyTcpClient::~EasyTcpClient()
{
	Close();
}


inline bool EasyTcpClient::InitSocket()
{
#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	//检测有连接就关闭旧连接
	if (INVALID_SOCKET != _sock)
	{
		cout << "关闭旧连接..." << endl;
		Close();
	}
	// 建立一个socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		cout << "错误，建立socket失败。" << endl;
		return false;
	}
	else
	{
		cout << "建立socket=<"<<_sock<<">成功。" << endl;
		return true;
	}
}

inline int EasyTcpClient::Connect(const char* ip, unsigned short port)
{
	//以防程序漏写InitSocket()
	if (!isRun())
	{
		InitSocket();
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);//Windows连接本机地址
#else
	_sin.sin_addr.s_addr = inet_addr(ip);//linux系统下连接Windows的IP地址
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "socket=<" << _sock << ">错误，连接服务器<"
			<< ip << "," << port << ">失败。" << endl;
	}
	else
	{
		cout << "socket=<" << _sock << ">连接服务器<"
			<< ip << "," << port << ">成功。" << endl;
	}
	return ret;
}

inline int EasyTcpClient::RcevData(SOCKET _cSock)
{
	//缓冲区
	char szRecv[4096] = {};
	int nLen = static_cast<int>(recv(_cSock, szRecv, sizeof(DataHeader), 0));
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "socket=<" << _cSock << ">与服务器断开连接，任务结束。" << endl;
		return -1;
	}
	//接受客户端请求，包头已经读取，偏移取包体其他信息
	recv(_cSock, szRecv + sizeof(DataHeader),
		header->dataLength - sizeof(DataHeader), 0);
	OnNetMsg(header);
	return 0;
}

inline int EasyTcpClient::SendData(DataHeader * header)
{
	if (isRun() && header)
	{
		return send(_sock, reinterpret_cast<const char*>(header),
			header->dataLength, 0);
	}
	return SOCKET_ERROR;
}


inline void EasyTcpClient::OnNetMsg(DataHeader* header)
{
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		//Login包含header所以把之前在缓存区的数据一并纳入login
		//LoginResult* login = reinterpret_cast<LoginResult*>(szRecv);
		cout << "socket=<" << _sock << ">收到服务器消息：CMD_LOGIN_RESULT, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		//LogoutResult* logout = reinterpret_cast<LogoutResult*>(szRecv);
		cout << "socket=<" << _sock << ">收到服务器消息：CMD_LOGOUT_RESULT, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		//NewUserJoin* userJoin = reinterpret_cast<NewUserJoin*>(szRecv);
		cout << "socket=<" << _sock << ">收到服务器消息：CMD_NEW_USER_JOIN, 数据长度："
			<< header->dataLength
			<< endl;
	}
	break;
	default:
	break;
	}
}
inline void EasyTcpClient::Close()
{
	if (_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_sock);
		//清除Windows socket环境
		WSACleanup();
#else
		//linux关闭
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}

}

inline bool EasyTcpClient::OnRun()
{
	if (isRun())
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		//{ 0,0 }非阻塞模式
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			cout << "socket=<" << _sock << ">select任务结束1" << endl;
			Close();
			return false;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == RcevData(_sock))
			{
				cout << "socket=<" << _sock << ">select任务结束2" << endl;
				Close();
				return false;
			}
		}
		return true;
		//cout << "空闲时间处理其他业务.." << endl;
	}
	return false;
}

#endif // !_EasyTcpClient_hpp_