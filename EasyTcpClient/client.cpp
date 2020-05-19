#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

using namespace std;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
//��Ϣͷ
struct DataHeader
{
	short dataLength;//���ݳ���
	short cmd;		//����
};
//DataPackage
struct Login
{
	char userName[32];
	char PassWord[32];
};
//���������Ƿ��½�ɹ�
struct LoginResult
{
	int result;
};
struct Logout
{
	char userName[32];
};
//�ǳ����
struct LogoutResult
{
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
			Login login{ "ljd","ljdmm" };
			DataHeader dh{sizeof(Login),CMD_LOGIN };
//5 ���������������
			send(_sock,(const char*)& dh, sizeof(DataHeader), 0);//��ͷ
			send(_sock, (const char*)& login, sizeof(Login), 0);//����
//6 ���շ�������Ϣ recv
			DataHeader retHeader{};
			LoginResult loginRet{};
			recv(_sock, (char*)& retHeader, sizeof(DataHeader), 0);
			recv(_sock, (char*)& loginRet, sizeof(LoginResult), 0);
			cout << "LoginResult:" << loginRet.result << endl;
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout{};
			DataHeader dh{ sizeof(Logout),CMD_LOGOUT };
//5 ���������������
			send(_sock, (const char*)& dh, sizeof(DataHeader), 0);//��ͷ
			send(_sock, (const char*)& logout, sizeof(Logout), 0);//����
//6 ���շ�������Ϣ recv
			DataHeader retHeader{};
			LogoutResult logoutRet{};
			recv(_sock, (char*)& retHeader, sizeof(DataHeader), 0);
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