#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;

	FILE *file;
	char fname[BUF_SIZE];
	char *fdata;
	int fsize;
	size_t result;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	clnt_adr_sz = sizeof(clnt_adr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	read(clnt_sock, &fname, BUF_SIZE);

	file = fopen(fname, "rt");
	if (file == NULL)
	{
		close(clnt_sock);
		close(serv_sock);
		error_handling("fopen() error");
	}

	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	rewind(file);

	fdata = (char*)malloc(sizeof(char) * fsize);
	result = fread((void*)fdata, 1, fsize, file);
	if (result != fsize)
	{
		close(clnt_sock);
		close(serv_sock);
		error_handling("fread() error");
	}

	write(clnt_sock, fdata, fsize);
	fclose(file);
	free(fdata);
	close(clnt_sock);
	close(serv_sock);

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}