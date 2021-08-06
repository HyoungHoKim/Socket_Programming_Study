#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in recv_adr;
	if (argc != 3)
	{
		printdf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&recv_adr, 0, sizeof(recv_adr));
	recv_adr.sin_family = AF_INET;
	recv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	recv_adr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&recv_adr, sizeof(recv_adr)) == -1)
		error_handling("connect() error!");
	
	// 데이터의 전송이 진행되고 있다. 단 MSG_OOB를 사용해서 긴급으로 데이터를 전송하는 부분이 
	// 있기 때문에 일반적인 도착순서인 123, 4, 567, 890의 순으로 전달되어야 하는데,
	// 이 중에서 4와 890이 긴급으로 전송되므로 도착순서에 변화가 생겼다고 예상해볼 수 있다. 
	write(sock, "123", strlen("123"));
	send(sock, "4", strlen("4"), MSG_OOB);
	write(sock, "567", strlen("567"));
	send(sock, "890", strlen("890"), MSG_OOB);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}