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

struct packet
{
	int len;  // 包头
        char buf[1024]; // 包体
};

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
	
	struct packet sendbuf;
	struct packet recvbuf;
	memset(&sendbuf,0,sizeof(sendbuf));
        memset(&recvbuf,0,sizeof(recvbuf));
	int n;
	while(fgets(sendbuf.buf,sizeof(sendbuf.buf),stdin)!=NULL){
	        n = strlen(sendbuf.buf);
		sendbuf.len = htonl(n); 	
		writen(sock,&sendbuf,4+n);  // 头部四个字节 + 实际数据
		
		int ret = readn(sock,&recvbuf.len,4); // 接受包头 确定长度
                if(ret == -1)  ERR_EXIT("read");
                  
                else if(ret < 4) { // 客户端关闭
                        printf("client close\n");
                        break;
                }
                 
                n =  ntohl(recvbuf.len);
                ret = readn(sock,&recvbuf.buf,n);
                if(ret == -1)  ERR_EXIT("read");
                else if(ret < n) { // 客户端关闭
                        printf("client close\n");
                        break;
                }
		fputs(recvbuf.buf,stdout);
		memset(&sendbuf,0,sizeof(sendbuf));
		memset(&recvbuf,0,sizeof(recvbuf));
	}
	close(sock);
	return 0;
}
