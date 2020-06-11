#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN    //尽量避免头文件中库中的冲突
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
	#include <sys/socket.h>
#endif

#include <iostream>
#include <vector>
#include "MessageHeader.hpp"

using namespace std;

class EasyTcpServer
{
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	
	//初始化socket
	SOCKET InitSocket()
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
			cout << "错误，建立socket=<" << _sock << ">失败。" << endl;
		}
		else
		{
			cout << "建立socket=<" << _sock << ">成功。" << endl;
		}
		return _sock;
	}
	
	//绑定IP和端口号
	int Bind(const char* ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //主机向网络转换host to net unsigned short
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}		
#endif
	//using namespace std之后，Mac系统默认bind为std::bind而不是::bind,会报错
		int ret = ::bind(_sock, reinterpret_cast<sockaddr*>(&_sin), sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			cout << "socket=<" << _sock << ">错误，绑定网络端口<" << port << ">失败。\n";
		}
		else
		{
			cout << "socket=<" << _sock << ">绑定网络端口<" << port << ">成功。\n";
		}
		return ret;
	}
	
	//监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			cout << "socket=<" << _sock << ">错误，监听网络端口失败。\n";
		}
		else
		{
			cout << "socket=<" << _sock << ">监听网络端口成功。\n";
		}
		return ret;
	}
	
	//接受客户端连接
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);
		//客户端_cSock,服务端_sock
#else
		_cSock = accept(_sock, reinterpret_cast<sockaddr*>(&clientAddr), reinterpret_cast<socklen_t *>(&nAddrLen));
#endif
		if (INVALID_SOCKET == _cSock)
		{
			cout << "socket = " << _cSock << ",错误，接收到无效客户端SOCKET。" << endl;
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			//inet_ntoa()功能是将网络地址转换成“.”点隔的字符串格式
			cout << "socket = <" << _sock << ">,新客户端加入：socket = " << _cSock << ",IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
		}
		return _cSock;
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
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
#else
			//清理已链接的客户端套接字
			for (size_t n = g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			//8 关闭套接字
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
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
			//最大描述符
			SOCKET maxSock = _sock;
			//每一次循环将队列中的客户端加入集合
			for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			timeval t = { 0,0 };
			//为兼容其他操作系统的 伯克利 socket,Windows下select()的第一个参数无意义;
			//select()第一个参数nfds是一个整数，是指fd_set集合中描述符最大值+1;
			//对应TCP协议中，接收方发送确认号是已发送数据序号的下一个。
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

			//ret=0无数据
			if (ret < 0)
			{
				cout << "select任务结束" << endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}

			for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						//处理成功后队列里移除此客户端
						auto iter = find(g_clients.begin(), g_clients.end(), g_clients[n]);
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
			//cout << "空闲时间处理其他业务.." << endl;
		}
		return false;
	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆包,包含OnNetMsg(header)操作
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[4096] = {};
		//5 接收客户端的请求,传来的包大于接收长度，后面的留到接下来接收
		int nLen = static_cast<int>(recv(_cSock, szRecv, sizeof(DataHeader), 0));
		DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
		if (nLen <= 0)
		{
			cout << "socket = <" << _sock 
				<< ">,客户端<Socket=" << _cSock 
				<< ">已退出，任务结束。" << endl;
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}

	//响应网络请求
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			//5 接受客户端请求，包头已经读取，偏移取包体其他信息
			
			//Login包含header所以把之前在缓存区的数据一并纳入login
			Login* login = reinterpret_cast<Login*>(header);
			cout << "socket = <" << _sock 
				<< ">,收到客户端<Socket=" << _cSock
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
			Logout* logout = reinterpret_cast<Logout*>(header);
			cout << "socket = <" << _sock 
				<< ">,收到客户端<Socket=" << _cSock
				<< ">请求：CMD_LOGOUT, 数据长度：" << header->dataLength
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
	//查询网络消息,包含RcevData(_sock)操作
	int SendData(SOCKET _cSock,DataHeader* header)
	{
		if (isRun() && header)
		{
			return static_cast<int>(send(_cSock, reinterpret_cast<const char*>(header),
				header->dataLength, 0));
		}
		return SOCKET_ERROR;
	}
	void SendDataToAll(DataHeader* header)
	{
		for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}
private:
	SOCKET _sock;
	vector<SOCKET> g_clients;//客户端
};

#endif // !_EasyTcpServer_hpp_
