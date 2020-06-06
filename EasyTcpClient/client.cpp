#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ

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

#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

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
	short cmd;		//����
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
		result = 0;	//Ĭ��0��������
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

int processor(SOCKET _cSock)
{
	//������
	char szRecv[1024] = {};
	//5 ���տͻ��˵�����,�����İ����ڽ��ճ��ȣ��������������������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = reinterpret_cast<DataHeader*>(szRecv);
	if (nLen <= 0)
	{
		cout << "��������Ͽ����ӣ����������" << endl;
		return -1;
	}
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{

		//5 ���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login����header���԰�֮ǰ�ڻ�����������һ������login
		LoginResult* login = reinterpret_cast<LoginResult*>(szRecv);
		cout << "�յ���������Ϣ��CMD_LOGIN_RESULT, ���ݳ��ȣ�"
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		//5 ���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login����header���԰�֮ǰ�ڻ�����������һ������login
		LogoutResult* logout = reinterpret_cast<LogoutResult*>(szRecv);
		cout << "�յ���������Ϣ��CMD_LOGOUT_RESULT, ���ݳ��ȣ�"
			<< header->dataLength
			<< endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		//5 ���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
		recv(_cSock, szRecv + sizeof(DataHeader),
			header->dataLength - sizeof(DataHeader), 0);
		//Login����header���԰�֮ǰ�ڻ�����������һ������login
		NewUserJoin* userJoin = reinterpret_cast<NewUserJoin*>(szRecv);
		cout << "�յ���������Ϣ��CMD_NEW_USER_JOIN, ���ݳ��ȣ�"
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

//�߳����б�־
bool g_bRun = true;

//�̺߳������ܲ������û����������
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			cout << "�˳�cmdThread�߳�" << endl;
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
			cout << "��֧�ֵ�����" << endl;
		}
	}

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
	//1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		cout << "���󣬽���socketʧ�ܡ�" << endl;
	}
	else
	{
		cout << "����socket�ɹ���" << endl;
	}
	//2 ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//Windows���ӱ�����ַ
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.74.1");//linuxϵͳ������Windows��IP��ַ
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "��������socketʧ�ܡ�";
	}
	else
	{
		cout << "����socket�ɹ���" << endl;
	}

	//�����߳�
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
			cout << "select�������1" << endl;
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				cout << "select�������2" << endl;
				break;
			}
		}
		//�߳�thread

		//cout << "����ʱ�䴦������ҵ��.." << endl;

	}
#ifdef _WIN32
	//7 �ر��׽���closesocket
	closesocket(_sock);
	//------------------
	//���Windows socket����
	WSACleanup();
#else
	//linux�ر�
	close(_sock);
#endif
	cout << "���˳���" << endl;
	getchar();
	return 0;
}