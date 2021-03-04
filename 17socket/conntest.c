
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h> 
#include<netinet/in.h>
#include<arpa/inet.h>

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>

#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)
// 测试连接最大数
int main(void){
	int count=0;
for(;;)
{	
	int sock;
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock<0) { 
		sleep(4);
		ERR_EXIT("socket");
	}
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	// 地址初始化
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	servaddr.sin_addr.s_addr = inet_addr("172.17.7.134");
	// inet_aton("172.17.7.134",&servaddr.sin_addr);
	// 连接
	if((connect(sock,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0) ERR_EXIT("connect");
//	 struct sockaddr_in localaddr;
//         socklen_t addrlen = sizeof(localaddr);
//         if(getsockname(sock,(struct sockaddr*)&loacladdr,&addrlen)<0) ERR_EXIT("getsockname");
//         printf("ip=%s port=%d\n",inet_ntoa(localaddr.sin_addr),ntohs(localaddr.sin_port));
	
	printf("连接数：%d\n",++count);
}
	return 0;
}
