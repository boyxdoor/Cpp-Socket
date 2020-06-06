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
#include <thread>

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
//��Ϣͷ
class DataHeader
{
public:
	short dataLength;//���ݳ���
	short cmd;        //����
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
//���������Ƿ��½�ɹ�
class LoginResult : public DataHeader
{
public:
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;    //Ĭ��0��������
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
//�ǳ����
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

//ȫ�ֵĿͻ����б�
vector<SOCKET> g_clients;

//��������
int processor(SOCKET _cSock)
{
	//������
	char szRecv[1024] = {};
	//5 ���տͻ��˵�����,�����İ����ڽ��ճ��ȣ��������������������
	int nLen = static_cast<int>(recv(_cSock, szRecv, sizeof(DataHeader), 0));
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "�ͻ���<Socket=" << _cSock << ">���˳������������" << endl;
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{

		//5 ���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login����header���԰�֮ǰ�ڻ�����������һ������login
		Login* login = reinterpret_cast<Login*>(szRecv);
		cout << "�յ��ͻ���<Socket=" << _cSock
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
		//���ܿͻ�������
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = reinterpret_cast<Logout*>(szRecv);
		cout << "�յ��ͻ���<Socket=" << _cSock
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
	return 0;
}

int main()
{
#ifdef _WIN32
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	//------------------
	//��socket API��������TCP������
	//1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2 bind �����ڽ��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); //����������ת��host to net unsigned short
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");��IP��ַ
#else
	_sin.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");��IP��ַ
#endif
	//using namespace std֮��MacϵͳĬ��bindΪstd::bind������::bind,�ᱨ��
	if (SOCKET_ERROR == ::bind(_sock, reinterpret_cast<sockaddr*>(&_sin), sizeof(sockaddr_in)))
	{
		cout << "���󣬰�����˿�ʧ�ܡ�\n";
	}
	else
	{
		cout << "������˿ڳɹ���\n";
	}
	//3 listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		cout << "���󣬼�������˿�ʧ�ܡ�\n";
	}
	else
	{
		cout << "��������˿ڳɹ���\n";
	}


	while (true)
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

		//ÿһ��ѭ���������еĿͻ��˼��뼯��
		for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		//���������
		SOCKET maxsock = _sock;
		for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
		{
			maxsock = max(g_clients[n], maxsock);
		}
		timeval t = { 1,0 };
		//Ϊ������������ϵͳ�� ������ socket,Windows��select()�ĵ�һ������������;
		//select()��һ������nfds��һ����������ָfd_set���������������ֵ+1;
		//��ӦTCPЭ���У����շ�����ȷ�Ϻ����ѷ���������ŵ���һ����
		int ret = select(maxsock + 1, &fdRead, &fdWrite, &fdExp, &t);

		//ret=0������
		if (ret < 0)
		{
			cout << "select�������" << endl;
			break;
		}
		//��鼯����ָ�����ļ��������Ƿ����
		if (FD_ISSET(_sock, &fdRead))
		{
			//��һ���������ļ��������Ӽ�����ɾ��
			FD_CLR(_sock, &fdRead);
			//4 accept �ȴ����տͻ�������
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
				cout << "���󣬽��յ���Ч�ͻ���SOCKET��" << endl;
			}
			else
			{
				for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n], reinterpret_cast<const char*>(&userJoin), sizeof(NewUserJoin), 0);
				}
				//�¿ͻ��˼�����������
				g_clients.push_back(_cSock);
				//inet_ntoa()�����ǽ������ַת���ɡ�.��������ַ�����ʽ
				cout << "�¿ͻ��˼��룺socket = " << _cSock << ",IP = " << inet_ntoa(clientAddr.sin_addr) << endl;
			}

		}

		for (int n = static_cast<int>(g_clients.size() - 1); n >= 0; n--)
		{
			if (FD_ISSET(g_clients[n], &fdRead))
			{
				if (-1 == processor(g_clients[n]))
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
		//cout << "����ʱ�䴦������ҵ��.." << endl;
	}
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
	cout << "���˳���" << endl;
	getchar();
	return 0;
}
