#define WIN32_LEAN_AND_MEAN	//��������ͷ�ļ��п��еĳ�ͻ
#include <WinSock2.h>
#include <Windows.h>

//#pragma comment(lib,"ws2_32.lib")	//��windowsƽ̨����

int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------------

	//------------------
	//���Windows socket����
	WSACleanup();
	return 0;
}