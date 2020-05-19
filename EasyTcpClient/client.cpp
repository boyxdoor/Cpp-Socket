#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

using namespace std;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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

int main()
{
//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
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
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "��������socketʧ�ܡ�";
	}
	else
	{
		cout << "����socket�ɹ���" << endl;
	}
	
	while (true)
	{
//3 ������������
		char cmdBuf[128] = {};
		cin >> cmdBuf;
//4 ��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			cout << "�յ��˳�������������" << endl;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			//5 ���������������
			Login login;
			strcpy(login.userName, "ljd");
			strcpy(login.PassWord, "ljdmm");
			//��������˰�ͷ
			send(_sock, (const char*)& login, sizeof(Login), 0);
//6 ���շ�������Ϣ recv
			LoginResult loginRet{};
			recv(_sock, (char*)& loginRet, sizeof(LoginResult), 0);
			cout << "LoginResult:" << loginRet.result << endl;
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "ljd");
//5 ���������������
			send(_sock, (const char*)& logout, sizeof(Logout), 0);//����
//6 ���շ�������Ϣ recv
			LogoutResult logoutRet{};
			recv(_sock, (char*)& logoutRet, sizeof(LogoutResult), 0);
			cout << "LogoutResult:" << logoutRet.result << endl;
		}
		else
		{
			cout << "��֧�ֵ�������������롣" << endl;
		}
	}

//7 �ر��׽���closesocket
	closesocket(_sock);
	//------------------
//���Windows socket����
	WSACleanup();
	cout << "���˳���" << endl;
	getchar();
	return 0;
}