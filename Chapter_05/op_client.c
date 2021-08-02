#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
// 피연산자의 바이트 수와 연산결과의 바이트 수를 상수화하였다.
#define RLT_SIZE 4
#define OPSZ 4

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	// 데이터의 송수신을 위한 메모리 공간은 이렇듯 배열을 기반으로 생성하는 것이 좋다.
	// 데이터를 누적해서 송수신해야하기 때문이다.
	char opmsg[BUF_SIZE];
	int result, opnd_cnt, i;
	struct sockaddr_in serv_adr;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
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
		error_handling("connect() error!");
	else
		puts("Connected.................");
	
	fputs("Operand count : ", stdout);
	scanf("%d", &opnd_cnt);
	opmsg[0] = (char)opnd_cnt;

	// 프로그램 사용자로부터 정수를 입력받아서 배열 opmsg에 이어서 저장하고 있다.
	// char형 배열 4바이트 int형 정수를 저장해야 하기 때문에 int형 포인터로 형변환을 하고 있다.
	// 만약에 이 부분이 이해되지 않는다면, 포인터에 대한 별도의 학습이 필요한 상태임을 인식하기 바란다.
	for (i = 0; i < opnd_cnt; i++)
	{
		printf("Operand %d : ", i + 1);
		scanf("%d", (int*)&opmsg[i * OPSZ + 1]);
	}
	// 문자를 입력받아야 하는데, 이에 앞서 버퍼에 남아있는 \n 삭제를 위해 fgetc 함수가 호출되고 있다. 
	fgetc(stdin);
	fputs("Operator : ", stdout);
	// 마지막으로 연산자 정보를 입력 받아서 배열 opmsg에 저장하고 있다.
	scanf("%c", &opmsg[opnd_cnt * OPSZ + 1]);
	// 드디어 write 함수호출을 통해서 opmsg에 저장되어 있는 연산과 관련된 정보를 한방에 전송하고 있다.
	// 이렇듯 한번의 write 함수 호출을 통해서 묶어도 되고, 여러 번의 write 함수 호출을 통해서 나눠서 보내도 된다.
	// 여러차례 강조했듯이 TCP는 데이터의 경계가 존재하지 않기 때문이다.
	write(sock, opmsg, opnd_cnt * OPSZ + 2);
	// 서버가 전송해주는 연산결과의 저장과정을 보이고 있다. 
	// 수신할 데이터의 크기가 4바이트이기 때문에 이렇게 한번의 read 함수 호출로 충분히 수신이 가능하다.
	read(sock, &result, RLT_SIZE);

	printf("Operation result : %d \n", result);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}