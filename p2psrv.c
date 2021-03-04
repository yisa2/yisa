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
	int listenfd;
	listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(listenfd<0) ERR_EXIT("socket");
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	// 地址初始化
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	// servaddr.sin_addr.s_addr = inet_addr("172.17.7.134");
	// inet_aton("172.17.7.134",&servaddr.sin_addr);
	//地址绑定 setsockopt 设置重复地址
	int on=1;
	if(setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) ERR_EXIT("setsockopt");
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0) ERR_EXIT("bind");
	// 监听
	if(listen(listenfd,SOMAXCONN)<0) ERR_EXIT("listen");
	
	// 接收
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn;
	if((conn=accept(listenfd,(struct sockaddr*)&peeraddr,&peerlen))<0)
		ERR_EXIT("accept");
	printf("ip=%s port=%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
	
	pid_t pid;
	pid = fork();
	if(pid==-1) ERR_EXIT("fork");
	if(pid == 0)
	{
		signal(SIGUSR1,handler);
		char sendbuf[1024] = {0};
		while(fgets(sendbuf,sizeof(sendbuf),stdin)!=NULL)
		{
			write(conn,sendbuf,strlen(sendbuf));
			memset(sendbuf,0,sizeof(sendbuf));
		}
		printf("child close\n");	
		exit(EXIT_SUCCESS);	
	}
	else
	{
		char recvbuf[1024];
		for(;;){
			memset(recvbuf,0,sizeof(recvbuf));
			int ret = read(conn,recvbuf,sizeof(recvbuf));
			if(ret==-1) ERR_EXIT("read");
			else if(ret==0) 
			{
				printf("peer close\n"); 
				break;
			}
			fputs(recvbuf,stdout);
		}
		printf("parent close\n");
		kill(pid,SIGUSR1);	
		exit(EXIT_SUCCESS);	
	}	
	close(listenfd);
	return 0;
}

