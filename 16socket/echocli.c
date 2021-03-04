#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<unistd.h> 
#include <sys/select.h>
#include<netinet/in.h>
#include <fcntl.h>
#include<signal.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

int read_timeout(int fd,unsigned int wait_seconds)
	 {
		 int ret=0;
		 if(wait_seconds > 0)
		 {
			 fd_set read_fdset;
			 struct timeval timeout;
			 
			 FD_ZERO(&read_fdset);
			 FD_SET(fd,&read_fdset);
			 
			 timeout.tv_sec = wait_seconds;
			 timeout.tv_usec = 0;
			 do{
				 ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);
			 }while(ret<0 && errno == EINTR);
			 if(ret == 0)
			 { 
				ret =-1;
				errno = ETIMEDOUT;
			  }
			  else if(ret == 1)
			  {
				  ret = 0;
			  }
		 }
		return ret;
	 }

int write_timeout(int fd,unsigned int wait_seconds)
 {
	 int ret=0;
	 if(wait_seconds > 0)
	 {
		 fd_set write_fdset;
		 struct timeval timeout;
		 
		 FD_ZERO(&write_fdset);
		 FD_SET(fd,&write_fdset);
		 
		 timeout.tv_sec = wait_seconds;
		 timeout.tv_usec = 0;
		 do{
			 ret = select(fd+1,NULL,&write_fdset,NULL,&timeout);
		 }while(ret<0 && errno == EINTR);
		 if(ret == 0)
		 { 
			ret =-1;
			errno = ETIMEDOUT;
		  }
		  else if(ret == 1)
		  {
			  ret = 0;
		  }
	return ret;
 }

int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
 {
	 int ret;
	 socklen_t addrlen = sizeof(struct sockaddr_in);  // 地址长度
	 
	 if(wait_seconds > 0)
	 {
		 fd_set accept_fdset;
		 struct timeval timeout;
		 
		 FD_ZERO(&accept_fdset);
		 FD_SET(fd,&accept_fdset);
		 
		 timeout.tv_sec = wait_seconds;
		 timeout.tv_usec = 0;
		 do{
			 ret = select(fd+1,&accept_timeout,NULL,NULL,&timeout);
		 }while(ret<0 && errno == EINTR);
		 if(ret == -1) return -1;
		 else if(ret == 0)
		 { 
			errno = ETIMEDOUT;
			return -1;
		  }
	 }
	 // 检测到事件不再阻塞
	 if(addr!=NULL) ret = accept(fd,(struct sockaddr*)addr,&addrlen);
	 else 
	 	ret = accept(fd,NULL,NULL);
         if(ret == -1)  ERR_EXIT("accept"); 	 	 		 	 	 
	 return ret;
 }
void activate_nonblock(int fd)
 {
	 int ret;
	 int flags = fcntl(fd,F_GETFL);
	 if(flags == -1)
		 ERR_EXIT("fcntl");
	 flags |= O_NONBLOCK;
	 ret = fcntl(fd,F_SETFL,flags);
	 if(ret == -1)
		 ERR_EXIT("fcntl");
 }
  void deactivate_nonblock(int fd)
 {
	 int ret;
	 int flags = fcntl(fd,F_GETFL);
	 if(flags == -1)
		 ERR_EXIT("fcntl");
	 flags &= ~O_NONBLOCK;
	 ret = fcntl(fd,F_SETFL,flags);
	 if(ret == -1)
		 ERR_EXIT("fcntl");
 }
 
 int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
 {
	 int ret;
	 socklen_t addrlen = sizeof(struct sockaddr_in);  // 地址长度
	 
	 
	 if(wait_seconds > 0)
	 {
		 activate_nonblock(fd);  // 将套接字改为非阻塞模式
	 }
	 
	 ret = connect(fd,(struct sockaddr*)addr,&addrlen);
	 if(ret < 0 && errno == EINPROGRESS) // EINPROGRESS 表示正在处理当中
	 {
		fd_set connect_fdset;
		 struct timeval timeout;
		 
		 FD_ZERO(&connect_fdset);
		 FD_SET(fd,&connect_fdset);
		 
		 timeout.tv_sec = wait_seconds;
		 timeout.tv_usec = 0;
		 do{
			 /* 一旦连接建立，套接字就可以写 */
			 ret = select(fd+1,NULL,&connect_fdset,NULL,&timeout);
		 }while(ret<0 && errno == EINTR);

		if(ret < -1) return -1;  // 连接
		 else if(ret == 0)   // 连接超时
		 { 
			errno = ETIMEDOUT;
			return -1;
		  }
		  else if(ret == 1)
		  {
			  /* ret 返回 1 可能出现两种情况，套接字连接成功、套接字产生错误
  			   * 此时错误信息不会保存至errno 变量中，因此需要调用getsockopt获取
  			   */

			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
			if(sockoptret == -1) return -1;
		      	if(err == 0) ret = 0;// 连接建立成功
		      	else  // 套接字产生错误
			 {
				  errno = err;
				  ret = -1;
			 }
		  }

	}
	if(wait_seconds>0)
	{
		 deactivate_nonblock(fd);
	}
	return ret;
}
#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

int main(void)
{
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
	
	int ret = connect_timeout(sock,&servaddr,5); // 超时时间设 5
	if(ret == -1 && errno == ETIMEDOUT)
	{
		printf("timeout...\n");
		return 1;
	}
	else if(ret == -1)
		ERR_EXIT("connect_timeout");
	
//	struct sockaddr_in localaddr;
//	socklen_t addrlen = sizeof(localaddr);
//	if(getsockname(sock,(struct sockaddr*)&localaddr,&addrlen)<0) 
//		ERR_EXIT("getsockname");
//	printf("ip=%s port=%d\n",inet_ntoa(loacladdr.sin_addr),ntohs(loacl.sin_port));

	return 0;
}
