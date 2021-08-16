#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

void exit_with_perror(const string &msg)
{
	cerr << msg << endl;
	exit(EXIT_FAILURE);
}

int main()
{
	int serv_sock;
	struct sockaddr_in serv_adr;

	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		exit_with_perror("socket() error\n" + string(strerror(errno)));
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(8080);
	if (::bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		exit_with_perror("bind() error\n" + string(strerror(errno)));
	
	if (listen(serv_sock, 5) == -1)
		exit_with_perror("listen() error\n" + string(strerror(errno)));
	fcntl(serv_sock, F_SETFL, O_NONBLOCK);

	// init kqueue
	int kq;
	if ((kq = kqueue()) == -1)
		exit_with_perror("kqueue() error\n" + string(strerror(errno)));
	
	map<int, string> clients; // map for client socket:data
	vector<struct kevent> change_list; // kevent vector for changelist
	struct kevent event_list[8]; // kevent array for eventlist

	// add event for server socket
	struct kevent temp_event;
	EV_SET(&temp_event, serv_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	change_list.push_back(temp_event);
	cout << "echo server started" << endl;

	// main loop
	int new_events;
	struct kevent *curr_event;
	while(1)
	{
		// apply changes and return new events(pending events)
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		if (new_events == -1)
			exit_with_perror("kevent() error\n" + string(strerror(errno)));
		
		change_list.clear(); // clear change_list for new changes

		for (int i = 0; i < new_events; ++i)
		{
			curr_event = &event_list[i];

			// check error event return
			if (curr_event->flags & EV_ERROR)
			{
				if (curr_event->ident == serv_sock)
					exit_with_perror("server socker error");
				else
				{
					cerr << "client socket error" << endl;
					cout << "client disconnected : " << curr_event->ident << endl;
					close(curr_event->ident);
					clients.erase(curr_event->ident);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == serv_sock)
				{
					// accept new client
					int clnt_sock;
					if ((clnt_sock = accept(serv_sock, NULL, NULL)) == -1)
						exit_with_perror("accept() error\n" + string(strerror(errno)));
					cout << "accept new client : " << clnt_sock << endl;
					fcntl(clnt_sock, F_SETFL, O_NONBLOCK);

					EV_SET(&temp_event, clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_list.push_back(temp_event);
					EV_SET(&temp_event, clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_list.push_back(temp_event);
					clients[clnt_sock] = "";
				}
				else if (clients.find(curr_event->ident) != clients.end())
				{
					// read data from client
					char buf[1024];
					int n = read(curr_event->ident, buf, sizeof(buf));

					if (n <= 0)
					{
						if (n < 0)
							cerr << "client read error!" << endl;
						cout << "client disconnected : " << curr_event->ident << endl;
						close(curr_event->ident);
						clients.erase(curr_event->ident);
					}
					else
					{
						buf[n] = '\0';
						clients[curr_event->ident] += buf;
						cout << "received data from " << curr_event->ident << " : " << clients[curr_event->ident] << endl;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				// send data to client
				map<int, string>::iterator it = clients.find(curr_event->ident);
				if (it != clients.end())
				{
					if (clients[curr_event->ident] != "")
					{
						int n;
                        if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(),
                                        clients[curr_event->ident].size()) == -1))
						{
							cerr << "client write error!" << endl;
							close(curr_event->ident);
							clients.erase(curr_event->ident);
						}
						else
							clients[curr_event->ident].clear();
					}
				}
			}
		}
	}
	return (0);
}