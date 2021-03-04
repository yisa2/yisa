// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<unistd.h> 
#include<netinet/in.h>
#include <sys/time.h>
 #include <sys/resource.h>

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

int main(void)
{
	struct rlimit rl;
	if(getrlimit(RLIMIT_NOFILE,&rl)<0) ERR_EXIT("getrlimit"); // 获取资源限制
	
	printf("%d\n",(int)rl.rlim_max);
	
	// 设置新的资源数
	rl.rlim_cur=2048;
	rl.rlim_max=2048;
	if(setrlimit(RLIMIT_NOFILE,&rl)<0) ERR_EXIT("setrlimit");
	if(getrlimit(RLIMIT_NOFILE,&rl)<0) ERR_EXIT("getrlimit"); // 获取资源限制
	
	printf("%d\n",(int)rl.rlim_max);
	
	return 0;	
}	
