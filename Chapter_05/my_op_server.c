#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);
int *get_num(char *message, int str_len);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	char message[BUF_SIZE];
	int *number;
	int i, str_len, cal;
	char ret[BUF_SIZE];

	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;

	if (argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("sock() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	clnt_adr_sz = sizeof(clnt_adr);

	for (int i = 0; i < 5; i++)
	{
		clnt_sock = accept(serv_sock,  (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		if (clnt_sock == -1)
			error_handling("accept() error");
		str_len = read(clnt_sock, message, BUF_SIZE);
		if (str_len == -1)
			error_handling("read() error!");
		printf("Check client data : %s", message);
		number = get_num(message, str_len - 1);
		cal = number[1];
		if (message[str_len - 2] == '+')
			for (i = 2; i <= number[0]; i++)
				cal += number[i];
		else if (message[str_len - 2] == '-')
			for (i = 2; i <= number[0]; i++)
				cal -= number[i];
		else if (message[str_len - 2] == '*')
			for (i = 2; i <= number[0]; i++)
				cal *= number[i];
		else if (message[str_len - 2] == '/')
			for (i = 2; i <= number[0]; i++)
				cal /= number[i];
		sprintf(ret, "%d", cal);
		printf("ret : %d\n", cal);
		write(clnt_sock, ret, strlen(ret));
		memset(message, 0, sizeof(message));
		free(number);
		close(clnt_sock);
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int *get_num(char *message, int str_len)
{
	int i = -1;
	int num = 0;
	int *save;
	int save_cnt = 0;

	while (isdigit(message[++i]))
		num = num * 10 + (message[i] - '0');
	save = (int*)malloc(sizeof(int) * num + 1);
	save[save_cnt++] = num;
	num = 0;
	while (++i < str_len)
	{
		if (isdigit(message[i]))
			num = num * 10 + (message[i] - '0');
		else
		{
			save[save_cnt++] = num;
			num = 0;
		}
	}
	return (save);
}