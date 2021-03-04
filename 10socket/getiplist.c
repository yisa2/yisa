// 回复服务器
#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>
#include <unistd.h>
#include<string.h>
#define ERR_EXIT(m)\
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)
int getlocalip(char *ip)
{
	char host[100] = {0};
	if(gethostname(host,sizeof(host))<0) return -1;
	struct hostent *hp;
        if((hp = gethostbyname(host)) == NULL) return -1;
        //strcpy(ip,inet_ntoa(*(struct in_addr*)hp->h_addr_list[0]));
        strcpy(ip,inet_ntoa(*(struct in_addr*)hp->h_addr));
	return 0;
}
int main(void)
{
	char ip[16] = {0};
	getlocalip(ip);
	printf("%s\n",ip);
	return 0;
}
