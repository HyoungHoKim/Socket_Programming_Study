#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds1[2], fds2[2];
    char str1[] = "A";
    char str2[] = "a";
    char buf[BUF_SIZE];
    pid_t pid;
	int i = 0;
    
    pipe(fds1), pipe(fds2);
    pid = fork();
	for (i = 0; i < 3; i++)
	{
		if (pid == 0)
		{
			str1[0] = str1[0] + i;
			write(fds1[1], str1, sizeof(str1));
			read(fds2[0], buf, BUF_SIZE);
			printf("Child proc output : %s \n", buf);
		}
		else
		{
			read(fds1[0], buf, BUF_SIZE);
			printf("Parent proc output : %s \n", buf);
			str2[0] = str2[0] + i;
			write(fds2[1], str2, sizeof(str2));
			// 큰 의미는 없다. 다만 부모 프로세스의 종료를 지연시키기 위해 삽입하였다. 
			sleep(3);
		}
	}
    return 0;
}