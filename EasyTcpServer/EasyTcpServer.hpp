#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN    //��������ͷ�ļ��п��еĳ�ͻ
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
	
	//��ʼ��socket
	SOCKET InitSocket()
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
			cout << "���󣬽���socket=<" << _sock << ">ʧ�ܡ�" << endl;
		}
		else
		{
			cout << "����socket=<" << _sock << ">�ɹ���" << endl;
		}
		return _sock;
	}
	
	//��IP�Ͷ˿ں�
	int Bind(const char* ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //����������ת��host to net unsigned short
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
	//using namespace std֮��MacϵͳĬ��bindΪstd::bind������::bind,�ᱨ��
		int ret = ::bind(_sock, reinterpret_cast<sockaddr*>(&_sin), sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			cout << "socket=<" << _sock << ">���󣬰�����˿�<" << port << ">ʧ�ܡ�\n";
		}
		else
		{
			cout << "socket=<" << _sock << ">������˿�<" << port << ">�ɹ���\n";
		}
		return ret;
	}
	
	//�����˿ں�
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			cout << "socket=<" << _sock << ">���󣬼�������˿�ʧ�ܡ�\n";
		}
		else
		{
			cout << "socket=<" << _sock << ">��������˿ڳɹ���\n";
		}
		return ret;
	}
	
	//���ܿͻ�������
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);
		//�ͻ���_cSock,�����_sock
#else
		_cSock = accept(_sock, reinterpret_cast<sockaddr*>(&clientAddr), reinterpret_cast<socklen_t *>(&nAddrLen));
#endif
		if (INVALID_SOCKET == _cSock)
		{
			cout << "socket = " << _cSock << ",���󣬽��յ���Ч�ͻ���SOCKET��" << endl;
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			//inet_ntoa()�����ǽ������ַת���ɡ�.��������ַ�����ʽ
			cout << "socket = <" << _sock << ">,�¿ͻ��˼��룺socket = " << _cSock << ",IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
		}
		return _cSock;
	}
	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//���������ӵĿͻ����׽���
			for (size_t n = g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}
			//8 �ر��׽���closesocket
			closesocket(_sock);
			//------------------
			//���Windows socket����
			WSACleanup();
#else
			//���������ӵĿͻ����׽���
			for (size_t n = g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			//8 �ر��׽���
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//socket ���ϵĽṹ��
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			//��ռ���
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			//���ļ�������_sock���뼯��֮��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			//���������
			SOCKET maxSock = _sock;
			//ÿһ��ѭ���������еĿͻ��˼��뼯��
			for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			timeval t = { 0,0 };
			//Ϊ������������ϵͳ�� ������ socket,Windows��select()�ĵ�һ������������;
			//select()��һ������nfds��һ����������ָfd_set���������������ֵ+1;
			//��ӦTCPЭ���У����շ�����ȷ�Ϻ����ѷ���������ŵ���һ����
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

			//ret=0������
			if (ret < 0)
			{
				cout << "select�������" << endl;
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
						//����ɹ���������Ƴ��˿ͻ���
						auto iter = find(g_clients.begin(), g_clients.end(), g_clients[n]);
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
			//cout << "����ʱ�䴦������ҵ��.." << endl;
		}
		return false;
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�������� ����ճ�� ���,����OnNetMsg(header)����
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[4096] = {};
		//5 ���տͻ��˵�����,�����İ����ڽ��ճ��ȣ��������������������
		int nLen = static_cast<int>(recv(_cSock, szRecv, sizeof(DataHeader), 0));
		DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
		if (nLen <= 0)
		{
			cout << "socket = <" << _sock 
				<< ">,�ͻ���<Socket=" << _cSock 
				<< ">���˳������������" << endl;
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}

	//��Ӧ��������
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			//5 ���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
			
			//Login����header���԰�֮ǰ�ڻ�����������һ������login
			Login* login = reinterpret_cast<Login*>(header);
			cout << "socket = <" << _sock 
				<< ">,�յ��ͻ���<Socket=" << _cSock
				<< ">����CMD_LOGIN, ���ݳ��ȣ�" << header->dataLength
				<< ", userName=" << login->userName
				<< ", PassWord=" << login->PassWord
				<< endl;
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LoginResult ret;
			//7 send ��ͻ��˷���һ������
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = reinterpret_cast<Logout*>(header);
			cout << "socket = <" << _sock 
				<< ">,�յ��ͻ���<Socket=" << _cSock
				<< ">����CMD_LOGOUT, ���ݳ��ȣ�" << header->dataLength
				<< ", userName=" << logout->userName
				<< endl;
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LogoutResult ret;
			//7 send ��ͻ��˷���һ������
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
	//��ѯ������Ϣ,����RcevData(_sock)����
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
	vector<SOCKET> g_clients;//�ͻ���
};

#endif // !_EasyTcpServer_hpp_
