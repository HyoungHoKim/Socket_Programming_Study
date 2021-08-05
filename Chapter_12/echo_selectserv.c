#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];
	if (argc != 2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	// select 함수의 두번째 인자로 전달될 fd_set형 변수 reads에 서버 소켓을 등록하고 있다.
	// 이로써 데이터의 수신여부를 관찰하는 관찰대상에 서버 소켓이 포함되어 있다. 
	// 참고로 클라이언트의 연결 요청도 데이터 요청을 통해서 이뤄진다. 
	// 따라서 이후에 서버소켓으로 수신된 데이터는가 존재한다는 것은 연결 요청이 있다는 뜻으로 해석해야한다. 
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;

	while (1)
	{
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		// while문으로 구성된 무한루프 내에서 select 함수가 호출되고 있다. select 함수의 세번째,
		// 그리고 네번째 매개변수가 비어있다. 이렇듯 관찰의 목적에 맞게 필요한 인자만 전달하면 된다. 
		if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;
		if (fd_num == 0)
			continue;
		
		for (i = 0; i < fd_max + 1; i++)
		{
			// select 함수가 1이상 반환했을 때 실행되는 반복문이다. 1 이상 반환되었으므로, 
			// 56행에서는 FD_ISSET 함수를 호출하면서 상태변화가 있었던(수신된 소켓의) 파일 디스크립터를 찾고 있다. 
			if (FD_ISSET(i, &cpy_reads))
			{
				// 상태 변화 확인되면 제일 먼저 서버 소켓에서 변화가 있었는지 확인한다. 그리고 서버 소켓의 상태 변화가 맞으면
				// 이어서 연결요청에 대한 수락의 과정을 진행한다.
				if (i == serv_sock)
				{
					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
					// fd_set형 변수 reads에 클라이언트와 연결된 소켓의 파일 디스크립터 정보를 등록함에 주의하자.
					FD_SET(clnt_sock, &reads);
					if (fd_max < clnt_sock)
						fd_max = clnt_sock;
					printf("connected client : %d \n", clnt_sock);
				}
				// 이어서 등장한 else문은 상태변화가 발생한 소켓이 서버 소켓이 아닌 경우에 실행된다. 
				// 즉, 수신할 데이터가 있는 경우에 실행된다. 단, 이 경우에도 수신한 데이터가 문자열 데이터인지,
				// 아니면 연결종료를 의미하는 EOF인지 확인해야한다. 
				else // read message!
				{
					str_len = read(1, buf, BUF_SIZE);
					// 수신한 데이터가 EOF인 경우에는 소켓을 종료하고 변수 reads에서 해당 정보를 삭제하는 과정을 거쳐야 한다. 
					if (str_len == 0) // close request
					{
						FD_CLR(i, &reads);
						close(i);
						printf("closed client : %d \n", i);
					}
					else
					{
						// 수신할 데이터가 문자열인 경우, 서버의 서비스 품목인 에코에 충실하기 위한 문장이다. 
						write(i, buf, str_len); // echo!
					}
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}