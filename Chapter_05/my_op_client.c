#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	char num[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;
	int i, j, cnt = 0;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error");
	else
		puts("Connected...............");
	
	fputs("Operand count : ", stdout);
	fgets(message, BUF_SIZE, stdin);
	cnt = atoi(message);
	i = -1;
	while (++i < cnt)
	{
		printf("Operand %d : ", i + 1);
		str_len = strlen(message);
		fgets(message + str_len, BUF_SIZE, stdin);
	}
	fputs("Operator : ", stdout);
	str_len = strlen(message);
	fgets(message + str_len, BUF_SIZE, stdin);

	write(sock, message, strlen(message));
	str_len = read(sock, message, BUF_SIZE - 1);
	message[str_len] = 0;
	printf("Operation result : %s \n", message);
	if (str_len == -1)
		error_handling("read() error!");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}