// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

void handler(int sig)
{
	printf("recv a sig=%d\n",sig);
	exit(EXIT_SUCCESS);	
}
int main(void){
	int sock;
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock<0) ERR_EXIT("socket");
	
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
	
	pid_t pid;
	pid = fork();
	if(pid==-1) ERR_EXIT("fork");
	if(pid==0)
	{
		char recvbuf[1024];
		for(;;){
			memset(recvbuf,0,sizeof(recvbuf));
			int ret = read(sock,recvbuf,sizeof(recvbuf));
			if(ret==-1) ERR_EXIT("read");
			else if(ret==0)
			{
				printf("peer close\n"); 
				break;
			}
			fputs(recvbuf,stdout);
		}
		close(sock);
		kill(getppid(),SIGUSR1);	
		exit(EXIT_SUCCESS);	
	}
	else
	{
		signal(SIGUSR1,handler);
		char sendbuf[1024]={0};
		while(fgets(sendbuf,sizeof(sendbuf),stdin)!=NULL)
		{
			write(sock,sendbuf,strlen(sendbuf));
			memset(sendbuf,0,sizeof(sendbuf));
		}
		close(sock);
	} 
	close(sock);
	return 0;
}
