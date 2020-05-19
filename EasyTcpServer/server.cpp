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
		//5 ���տͻ��˵�����
		int nLen = recv(_cSock, (char*)& header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			cout << "�ͻ������˳������������" << endl;
			break;
		}
		cout << "�յ����" << header.cmd<<", ���ݳ��ȣ�"
			<<header.dataLength<<endl;
		//6 ��������
		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			Login login{};
			//���ܿͻ�������
			recv(_cSock, (char*)&login, sizeof(Login), 0);
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LoginResult ret{0};
			//������Ϣͷ
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			//���ص�¼���
			send(_cSock, (char*)& ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout logout{};
			//���ܿͻ�������
			recv(_cSock, (char*)&logout, sizeof(Logout), 0);
			//�����ж��û������Ƿ���ȷ�Ĳ�����
			LogoutResult ret{ 1 };
			////7 send ��ͻ��˷���һ������ ������Ϣͷ
			send(_cSock, (const char *)& header, sizeof(DataHeader), 0);
			//���ص�¼���
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
