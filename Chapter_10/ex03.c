#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;

	pid_t pid;
	int status;
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	pid = fork();
	if (pid == 0)
	{
		printf("Child socket fp : %d \n", sock);
		close(sock);
		return 0;
	}

	waitpid(pid, &status, 0);
	printf("Parent socket fp : %d \n", sock);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}