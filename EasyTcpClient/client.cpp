#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

using namespace std;
struct DataPackage
{
	int age;
	char name[32];
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
		else
		{
//5 ���������������
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}
//6 ���շ�������Ϣ recv
		char recvBuf[128]{};
		int nlen = recv(_sock, recvBuf, 128, 0);
		if (nlen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			cout << "���յ����ݣ�����="<<dp->age<<", ����="<<dp->name<<endl;
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