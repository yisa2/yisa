// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<unistd.h> 
#include<netinet/in.h>
#include<signal.h>
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

size_t recv_peek(int sockfd, void *buf, size_t len)
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
		for( i=0;i<nread;i++)
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
void echo_cli(int sock)
{
/*
	char  sendbuf[1024] = {0};
	char  recvbuf[1024] = {0};
	while(fgets(sendbuf,sizeof(sendbuf),stdin)!=NULL){ 	
		writen(sock,sendbuf+1,strlen(sendbuf)); 
		
		int ret = readline(sock,recvbuf,sizeof(recvbuf)); 
                if(ret == -1)  ERR_EXIT("readline");
                else if(ret ==0 ) { // 客户端关闭
                        printf("client close\n");
                        break;
                }
		fputs(recvbuf,stdout);
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sock);
*/
	// select
	fd_set rset;
	FD_ZERO(&rset);
	
	int nready; // 检测到的事件个数
	int maxfd;
	int fd_stdin = fileno(stdin);  // 获取当前文件描述符
	if(fd_stdin > sock) maxfd = fd_stdin;
	else maxfd = sock;
	char  sendbuf[1024] = {0};
	char  recvbuf[1024] = {0};
	for(;;)
	{
		// 将文件描述符加入集合
		FD_SET(fd_stdin,&rset);
		FD_SET(sock,&rset);
		// select 返回感兴趣事件
		nready = select(maxfd+1,&rset,NULL,NULL,NULL);
		if(nready == -1) ERR_EXIT("select");
		if(nready == 0) continue;  // 超时
		// 检测是否发生可读事件
		if(FD_ISSET(sock,&rset))
		{
			int ret = readline(sock,recvbuf,sizeof(recvbuf)); 
                	if(ret == -1)  ERR_EXIT("readline");
                	else if(ret ==0 ) { // 客户端关闭
                        	printf("server close\n");
                        	break;
                	}
			fputs(recvbuf,stdout);
			memset(recvbuf,0,sizeof(recvbuf));
		}
		if(FD_ISSET(fd_stdin,&rset))
		{
			if(fgets(sendbuf,sizeof(sendbuf),stdin)==NULL) break;
			writen(sock,sendbuf,strlen(sendbuf));
			memset(recvbuf,0,sizeof(recvbuf));	
		}	
		
	}
	close(sock);
}
void handle_sigpie(int sig)
{
	printf("recv a sig = %d\n",sig);
}
int main(void){
	signal(SIGPIPE,handle_sigpie);
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
	echo_cli(sock);
	return 0;
}
