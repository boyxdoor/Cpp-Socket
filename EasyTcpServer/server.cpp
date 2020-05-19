#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
class Login: public DataHeader
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
	//��socket API��������TCP������
	//1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2 bind �����ڽ��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); //����������ת��host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");��IP��ַ
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		cout << "���󣬰�����˿�ʧ�ܡ�\n";
	}
	else
	{
		cout << "������˿ڳɹ���\n";
	}
	//3 listen ��������˿�
	if(SOCKET_ERROR==listen(_sock, 5))
	{
		cout << "���󣬼�������˿�ʧ�ܡ�\n";
	}
	else
	{
		cout << "��������˿ڳɹ���\n";
	}
	//4 accept �ȴ����տͻ�������
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;

	//�ͻ���_cSock,�����_sock
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		cout << "���󣬽��յ���Ч�ͻ���SOCKET��"<<endl ;
	}
	//inet_ntoa()�����ǽ������ַת���ɡ�.��������ַ�����ʽ
	cout << "�¿ͻ��˼��룺socket = " << _cSock<<",IP = "<<inet_ntoa(clientAddr.sin_addr) << endl;
	char _recvBuf[128] = {};

	while (true)
	{
		DataHeader header{};
		//5 ���տͻ��˵�����,�����İ����ڽ��ճ��ȣ��������������������
		int nLen = recv(_cSock, (char*)& header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			cout << "�ͻ������˳������������" << endl;
			break;
		}
		//6 ��������
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login{};
			//���ܿͻ������󣬰�ͷ�Ѿ���ȡ��ƫ��ȡ����������Ϣ
			recv(_cSock, (char*)&login+sizeof(DataHeader), 
				sizeof(Login)- sizeof(DataHeader), 0);
			cout << "�յ����CMD_LOGIN"  
				<< ", ���ݳ��ȣ�"<< header.dataLength 
				<< ", userName="<<login.userName
				<<", PassWord="<<login.PassWord
				<<endl;
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LoginResult ret;
			//7 send ��ͻ��˷���һ������
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout logout{};
			//���ܿͻ�������
			recv(_cSock, (char*)&logout + sizeof(DataHeader), 
				sizeof(Logout) - sizeof(DataHeader), 0);
			cout << "�յ����CMD_LOGOUT"
				<< ", ���ݳ��ȣ�" << header.dataLength
				<< ", userName=" << logout.userName
				<< endl;
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LogoutResult ret;
			//7 send ��ͻ��˷���һ������
			send(_cSock, (char*)& ret, sizeof(LogoutResult), 0);
		}
		break;
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			break;
		}
	}
	
	//8 �ر��׽���closesocket
	closesocket(_sock);
	//------------------
	//���Windows socket����
	WSACleanup();
	cout << "���˳���" << endl;
	getchar();
	return 0;
}
