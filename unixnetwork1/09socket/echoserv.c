// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<unistd.h>
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
struct packet
{
	int len;  // 包头
	char buf[1024]; // 包体
};
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
void do_service(int conn)
{
	struct packet recvbuf;
	int n;
	for(;;){
		memset(&recvbuf,0,sizeof(recvbuf));
		int ret = readn(conn,&recvbuf.len,4); // 接受包头 确定长度
		if(ret == -1)  ERR_EXIT("read");

		else if(ret < 4) { // 客户端关闭
			printf("client close\n");
			break;
		}
		
		n =  ntohl(recvbuf.len);
		ret = readn(conn,&recvbuf.buf,n);
		if(ret == -1)  ERR_EXIT("read");
		else if(ret < n) { // 客户端关闭
			printf("client close\n");
			break;
		}
		fputs(recvbuf.buf,stdout);
		write(conn,&recvbuf,4+n);
	}
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
			do_service(conn);
			exit(EXIT_SUCCESS);
		}
		else close(conn); 
	}
	return 0;
}

