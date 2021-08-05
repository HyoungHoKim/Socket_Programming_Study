#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void Quit(int sig)
{
	char message[30];

    if (sig == SIGINT)
	{
    	puts("Really Quit program? (press y or Y) : ");
		fgets(message, sizeof(message), stdin);
		if (!strcmp(message, "y\n") || !strcmp(message, "Y\n"))
			exit(1);
	}
}

int main(int argc, char *argv[])
{
    int i;
    struct sigaction act;
    act.sa_handler = Quit;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    
	while(1)
	{
		puts("!");
		sleep(1);
	}
    return 0;
}