#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN    //��������ͷ�ļ��п��еĳ�ͻ
	#include <WinSock2.h>
	#include <Windows.h>
	#pragma comment(lib,"ws2_32.lib")    //��windowsƽ̨����
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

	//��ʼ��socket
	bool InitSocket();

	//���ӷ�����
	int Connect(const char* ip, unsigned short port);

	// �ر��׽���closesocket
	void Close();

	//�������� ����ճ�� ���,����OnNetMsg(header)����
	int RcevData(SOCKET _sock);

	//��������,�ͻ����н����̴߳���
	int SendData(DataHeader* header);

	//��Ӧ��������
	void OnNetMsg(DataHeader* header);

	//��ѯ������Ϣ,����RcevData(_sock)����
	bool OnRun();

	//�Ƿ�����
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
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	//��������Ӿ͹رվ�����
	if (INVALID_SOCKET != _sock)
	{
		cout << "�رվ�����..." << endl;
		Close();
	}
	// ����һ��socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		cout << "���󣬽���socketʧ�ܡ�" << endl;
		return false;
	}
	else
	{
		cout << "����socket=<"<<_sock<<">�ɹ���" << endl;
		return true;
	}
}

inline int EasyTcpClient::Connect(const char* ip, unsigned short port)
{
	//�Է�����©дInitSocket()
	if (!isRun())
	{
		InitSocket();
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);//Windows���ӱ�����ַ
#else
	_sin.sin_addr.s_addr = inet_addr(ip);//linuxϵͳ������Windows��IP��ַ
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "socket=<" << _sock << ">�������ӷ�����<"
			<< ip << "," << port << ">ʧ�ܡ�" << endl;
	}
	else
	{
		cout << "socket=<" << _sock << ">���ӷ�����<"
			<< ip << "," << port << ">�ɹ���" << endl;
	}
	return ret;
}

inline int EasyTcpClient::RcevData(SOCKET _cSock)
{
	//������
	char szRecv[4096] = {};
	int nLen = static_cast<int>(recv(_cSock, szRecv, sizeof(DataHeader), 0));
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "socket=<" << _cSock << ">��������Ͽ����ӣ����������" << endl;
		return -1;
	}
	//���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
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
		//Login����header���԰�֮ǰ�ڻ�����������һ������login
		//LoginResult* login = reinterpret_cast<LoginResult*>(szRecv);
		cout << "socket=<" << _sock << ">�յ���������Ϣ��CMD_LOGIN_RESULT, ���ݳ��ȣ�"
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		//LogoutResult* logout = reinterpret_cast<LogoutResult*>(szRecv);
		cout << "socket=<" << _sock << ">�յ���������Ϣ��CMD_LOGOUT_RESULT, ���ݳ��ȣ�"
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		//NewUserJoin* userJoin = reinterpret_cast<NewUserJoin*>(szRecv);
		cout << "socket=<" << _sock << ">�յ���������Ϣ��CMD_NEW_USER_JOIN, ���ݳ��ȣ�"
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
		//���Windows socket����
		WSACleanup();
#else
		//linux�ر�
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
		//{ 0,0 }������ģʽ
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			cout << "socket=<" << _sock << ">select�������1" << endl;
			Close();
			return false;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == RcevData(_sock))
			{
				cout << "socket=<" << _sock << ">select�������2" << endl;
				Close();
				return false;
			}
		}
		return true;
		//cout << "����ʱ�䴦������ҵ��.." << endl;
	}
	return false;
}

#endif // !_EasyTcpClient_hpp_