#pragma comment(lib,"Ws2_32.lib" )

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#define MCASTADDR "233.0.0.55" //本例使用的多播组地址。
#define MCASTPORT 5155 //绑定的本地端口号。
#define BUFSIZE 1024 //接收数据缓冲大小。

int main(int argc, char ** argv)
{
	WSADATA wsd;
	struct sockaddr_in local, remote, from;
	SOCKET sock, sockM;
	TCHAR recvbuf[BUFSIZE];
	/*struct ip_mreq mcast; // Winsock1.0 */
	int len = sizeof(struct sockaddr_in);
	int ret;
	//初始化WinSock2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("WSAStartup() failed\n");
		return -1;
	}
	/*
	创建一个SOCK_DGRAM类型的SOCKET
	其中,WSA_FLAG_MULTIPOINT_C_LEAF表示IP多播在控制面层上属于
	"无根"类型;
	WSA_FLAG_MULTIPOINT_D_LEAF表示IP 多播在数据面层上属于"无根",
	有关控制面层和
	数据面层有关概念请参阅MSDN说明。
	*/
	if ((sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
		WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("socket failed with:%d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	//将sock绑定到本机某端口上。
	local.sin_family = AF_INET;
	local.sin_port = htons(MCASTPORT);
	local.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind failed with:%d \n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	//加入多播组
	remote.sin_family = AF_INET;
	remote.sin_port = htons(MCASTPORT);
	remote.sin_addr.s_addr = inet_addr(MCASTADDR);
	/* Winsock1.0 */
	/*
	mcast.imr_multiaddr.s_addr = inet_addr(MCASTADDR);
	mcast.imr_interface.s_addr = INADDR_ANY;
	if( setsockopt(sockM,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mcast,
	sizeof(mcast)) == SOCKET_ERROR)
	{
	printf("setsockopt(IP_ADD_MEMBERSHIP) failed:%d\n",WSAGetLastError()); closesocket(sockM);
	WSACleanup();
	return -1;
	}
	*/
	/* Winsock2.0*/
	if ((sockM = WSAJoinLeaf(sock, (SOCKADDR*)&remote, sizeof(remote),
		NULL, NULL, NULL, NULL,
		JL_BOTH)) == INVALID_SOCKET)
	{
		printf("WSAJoinLeaf() failed:%d\n", WSAGetLastError()); closesocket(sock);
		WSACleanup();
		return -1;
	}
	//接收多播数据，当接收到的数据为"QUIT"时退出。
	while (1)
	{
		if ((ret = recvfrom(sock, recvbuf, BUFSIZE, 0,
			(struct sockaddr*)&from, &len)) == SOCKET_ERROR)
		{
			printf("recvfrom failed with:%d\n", WSAGetLastError()); closesocket(sockM);
			closesocket(sock);
			WSACleanup();
			return -1;
		}
		if (strcmp(recvbuf, "QUIT") == 0) break;
		else
		{
			recvbuf[ret] = '\0';
			printf("%s\n", recvbuf);
		}
	}
	closesocket(sockM);
	closesocket(sock);
	WSACleanup();
	return 0;
}