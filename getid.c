#include<unistd.h>
#include<stdio.h>
int main()
{
	uid_t uid = getuid(), euid = geteuid();
	printf("uid:%d;euid:%d\n",uid,euid);
	return 0;
}

