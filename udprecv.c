#include <winsock.h>
#include <stdio.h>

#define PORT 27960

void main()
{
	WSADATA data;
	struct sockaddr_in sin;
	SOCKET s;
	int retval, size = sizeof(sin);
	char* buffer;

	WSAStartup(MAKEWORD(1,1),&data);

	buffer = (char*)calloc(data.iMaxUdpDg,sizeof(char));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);
	memset(sin.sin_zero,0,8);

	printf("Waiting for UDP packets on port %d...\n",PORT);

	s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	if (s == INVALID_SOCKET)
	{
		perror("socket()");
	}
	else
	{
		retval = bind(s,(struct sockaddr*)&sin,size);

		if (retval == SOCKET_ERROR)
		{
			perror("bind()");
		}
		else
		{
			while ((retval = recvfrom(s,buffer,data.iMaxUdpDg,0,(struct sockaddr*)&sin,&size)) != SOCKET_ERROR)
			{
				if (retval > 0)
				{
					printf("Packet recieved from IP address %s, size: %d\n",inet_ntoa(sin.sin_addr),retval);
				}
			}
		}
	}

	free(buffer);
	WSACleanup();
}