#include "EasyTcpClient.hpp"
#include <iostream>
#include <thread>


using namespace std;


//�߳����б�־

//�̺߳������ܲ������û����������
void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			cout << "�˳�cmdThread�߳�" << endl;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "ljd");
			strcpy(login.PassWord, "ljd");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "ljd");
			client->SendData(&logout);
		}
		else
		{
			cout << "��֧�ֵ�����" << endl;
		}
	}

}

int main()
{
	EasyTcpClient client1;
	//client1.InitSocket();
	client1.Connect("127.0.0.1",4567);
	//�����߳�
	thread t1(cmdThread, &client1);
	t1.detach();

	EasyTcpClient client2;
	//client2.InitSocket();
	client2.Connect("127.0.0.1", 4568);
	thread t2(cmdThread, &client2);
	t2.detach();

	while (client1.isRun() || client2.isRun())
	{
		client1.OnRun();
		client2.OnRun();
	}
	client1.Close();
	client2.Close();

	cout << "���˳���" << endl;

	getchar();

	return 0;
}
