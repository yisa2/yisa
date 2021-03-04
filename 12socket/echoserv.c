// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<signal.h>
#include<unistd.h>
#include <sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)
ssize_t writen(int fd, const void *buf, size_t count)
{	
	size_t nleft = count;
	ssize_t nwritten;          
	char *bufp = (char*)buf;
	while(nleft>0)
	{
		if((nwritten=write(fd,bufp,nleft))<0)
		{
			if(errno == EINTR) continue;
			return -1;
		}
		else if(nwritten == 0) continue;
		
		bufp += nwritten;  // 指针偏移
		nleft -= nwritten;
	}
	return count;
}
size_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;          
	char *bufp = (char*)buf;
	while(nleft>0)
	{
		if((nread=read(fd,bufp,nleft))<0)
		{
			if(errno == EINTR) continue;
			return -1;
		}
		else if(nread == 0) return count - nleft; //表示对方关闭  已经读取的字节数
		
		bufp += nread;  // 指针偏移
		nleft -= nread;
	}
	return count; // 全部字节数
}
ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	for(;;)
	{
		int ret = recv(sockfd,buf,len,MSG_PEEK);
		if(ret == -1 && errno == EINTR) continue;
		return ret;
	}
}
// 使用recv 函数实现readline 只适用于套接口
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = buf;
	int nleft = maxline;
	for(;;)
	{
		ret = recv_peek(sockfd,bufp,nleft);
		if(ret<0) return ret;  // 失败
		else if(ret==0) return ret; // 对等方关闭
		nread = ret;
		int i;
		for(i=0;i<nread;i++)
		{
			if(bufp[i]=='\n')
			{
				ret = readn(sockfd,bufp,i+1);
				if(ret != i+1) exit(EXIT_FAILURE); // 失败
				return ret;
			}
		}
		if(nread>nleft) exit(EXIT_FAILURE);
		
		nleft -= nread;
		ret = readn(sockfd,bufp,nread);
		if(ret != nread) exit(EXIT_FAILURE);
		bufp += nread;	
	}
	return -1;
}
void echo_srv(int conn)
{
	char recvbuf[1024];
	for(;;){
		memset(recvbuf,0,sizeof(recvbuf));
		int ret = readline(conn,recvbuf,1024); // 接受包头 确定长度
		if(ret == -1)  ERR_EXIT("readline");

		if(ret == 0) 
		{
			printf("client close\n");
			break;
		}
		
		fputs(recvbuf,stdout);
		write(conn,recvbuf,strlen(recvbuf));
	}
}
void handle_sigchld(int sig)
{
//	wait(NULL);
	while(waitpid(-1,NULL,WNOHANG)>0);
}
int main(void){
//	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,handle_sigchld);	
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
	pid_t pid;
	for(;;)
	{
		if((conn=accept(listenfd,(struct sockaddr*)&peeraddr,&peerlen))<0)
			ERR_EXIT("accept");
		printf("ip=%s port=%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
		
		pid = fork();
		if(pid==-1) ERR_EXIT("fork");
		if(pid==0){
			close(listenfd);
			echo_srv(conn);
			exit(EXIT_SUCCESS);
		}
		else close(conn); 
	}
	return 0;
}

