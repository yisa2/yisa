/* UDP 数据报截断例子 */
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

int main(void)
{
	int sock;
	if((sock = socket(PF_INET,SOCK_DGRAM,0))<0) ERR_EXIT("socket");	
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0) ERR_EXIT("bind");
	
	sendto(sock,"ABCD",4,0,(struct sockaddr*)&servaddr,sizeof(servaddr)); // 给自己发送消息
	char recvbuf[1] = {0};
	int i,n;
	for(i=0; i<4;i++)
	{
		n = recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL);
		if(n == -1)
		{
			if(errno == EINTR) continue;
			ERR_EXIT("recvfrom");	
		}
		else if(n>0)
		{
			printf("n=%d %c\n",n,recvbuf[0]);	
		}
	}
	return 0;	
}

/* 发现出现数据丢失，如果接受的缓冲区大小小于数据报大小，只接受缓冲区大小的数据，其他丢失，数据截断 */

