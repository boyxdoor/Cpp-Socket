#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

using namespace std;

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
		//5 ���տͻ��˵�����
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			cout << "�ͻ������˳������������" << endl;
			break;
		}
		cout << "�յ����" << _recvBuf<<endl;
		//6 ��������
		if (0 == strcmp(_recvBuf, "getName"))
		{
			//7 send ��ͻ��˷���һ������
			const char msgBuf[] = "Xiao Liang";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//��β��һ������ȥ
		}
		else if (0 == strcmp(_recvBuf, "getAge"))
		{
			//7 send ��ͻ��˷���һ������
			const char msgBuf[] = "80";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//��β��һ������ȥ
		}
		else
		{
			//7 send ��ͻ��˷���һ������
			const char msgBuf[] = "???";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);//��β��һ������ȥ
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
