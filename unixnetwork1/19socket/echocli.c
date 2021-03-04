#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

void echo_cli(int sock)
{
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = inet_addr("172.17.7.134");
	
        connect(sock,(struct sockaddr*)&servaddr,sizeof(servaddr));	
	int ret;	
	char recvbuf[1024] = {0};
	char sendbuf[1024] = {0};
	while(fgets(sendbuf,sizeof(sendbuf),stdin)!=NULL)
	{
	/*	sendto(sock,sendbuf,strlen(sendbuf),0,(struct sockaddr*)&servaddr,sizeof(servaddr)); */
	/*	sendto(sock,sendbuf,strlen(sendbuf),0,NULL,0);*/
		send(sock,sendbuf,strlen(sendbuf),0);
		ret = recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL);
		if(ret == -1 && errno == EINTR) continue;
		else if(ret == -1) ERR_EXIT("recvfrom");
		fputs(recvbuf,stdout);
		memset(recvbuf,0,sizeof(recvbuf));
		memset(sendbuf,0,sizeof(sendbuf));
	}
}
int main(void)
{
	int sock;
	if((sock = socket(PF_INET,SOCK_DGRAM,0))<0) ERR_EXIT("socket");	
	echo_cli(sock);	
	
	return 0;
}
