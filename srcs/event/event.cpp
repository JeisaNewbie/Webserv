#include "../core/core.hpp"

static void prepConnect(Cycle& cycle);
static void addEvent(int kq, uintptr_t ident, int16_t filter,			\
						uint16_t flags,	uint32_t fflags,						\
						intptr_t data, std::string* udata);
static void acceptNewClient(Cycle& cycle, int kq, uintptr_t listen_socket, std::map<int, Client>& server);
static bool recieveFromClient(Client& client);
static bool sendToClient(Client& client);
static void disconnectClient(int client_socket);
static void checkCgiForkList(std::vector<Client*>& cgi_fork_list);

// 삭제하기
void printState(kevent_t* cur_event) {
	std::cout << "client[" << cur_event->ident << "]\n";
	if (cur_event->flags & EV_EOF)
		std::cout << "EOF\n";
	if (cur_event->flags & EV_ERROR)
		std::cout << "ERROR\n";
	if (cur_event->flags & EV_ADD)
		std::cout << "ADD\n";
	if (cur_event->filter == EVFILT_READ)
		std::cout << "READ\n";
	if (cur_event->filter == EVFILT_WRITE)
		std::cout << "WRITE\n";
	if (errno == EAGAIN)
		std::cout << "EAGAIN\n";
	if (errno == ECONNRESET)
		std::cout << "ECONNRESET\n";
	std::cout << "\n";
}

void test(int kq) {
    kevent_t tmp[2];
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;
    int new_event = kevent(kq, NULL, 0, tmp, 2, &timeout);
    std::cout << "\n\ntest: " << new_event << "\n";
    printState(&tmp[0]);
    printState(&tmp[1]);
    std::cout << "\n\n\n";
}

static void checkReadEventList(std::vector<Client*>& read_event_list, Cycle& cycle, int kq, uintptr_t* cgi_fd_arr, std::vector<Client*>& cgi_fork_list) {
	for (int i = 0; i < read_event_list.size(); i++) {
		if (read_event_list[i]->get_timeout_instance().checkTimeout() == false) {
			if (recieveFromClient(*read_event_list[i]) == false) {
				read_event_list.erase(read_event_list.begin() + i--);
				continue;
			}
		}
		else {
			read_event_list.erase(read_event_list.begin() + i--);
			// request parsing
			read_event_list[i]->do_parse(cycle);
			read_event_list[i]->get_response_instance().set_body("");
			// read_event_list[i]->get_request_instance().check_members();
			if (read_event_list[i]->get_status_code() < MOVED_PERMANENTLY && read_event_list[i]->get_expect() == false)
			{
				try
				{
					if (read_event_list[i]->get_cgi() == true)
					{
						read_event_list[i]->set_property_for_cgi(read_event_list[i]->get_request_instance());
						uintptr_t	fd = read_event_list[i]->get_cgi_instance().get_fd();

						addEvent(kq, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, cycle.getEventTypeCgi());
						cgi_fd_arr[fd] = read_event_list[i]->get_client_soket();
						cgi_fork_list.push_back(read_event_list[i]);

						continue;
					}
					read_event_list[i]->do_method_without_cgi(read_event_list[i]->get_request_instance());
				}
				catch(int e)
				{
					read_event_list[i]->set_status_code(e);
				}
			}
			read_event_list[i]->assemble_response();
			read_event_list[i]->get_request_instance().get_request_msg() = "";
			addEvent(kq, read_event_list[i]->get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, cycle.getEventTypeClient());
			std::cout << "---------------end of assebling message--------------\n";
		}
	}
}

void startConnect(Cycle& cycle) {
    Worker					worker;
    std::vector<uintptr_t>&	listen_socket_list = cycle.getListenSocketList();
    std::vector<kevent_t>	event_list(cycle.getWorkerConnections() * 2);
    std::map<int, Client>	server;
    uintptr_t				cgi_fd_arr[MAX_FD] = {0,};
    std::vector<Client*>	read_event_list;
    std::vector<Client*>	cgi_fork_list;

    prepConnect(cycle);
    std::cout << "---------------------webserver start---------------------\n";

    for (int i = 0; i < listen_socket_list.size(); i++)
        addEvent(worker.getEventQueue(), listen_socket_list[i], EVFILT_READ, EV_ADD, 0, 0, cycle.getEventTypeListen());

	uint32_t		new_events;
	kevent_t*		cur_event;
	struct timespec	kevent_timeout;

	while (1) {
		kevent_timeout.tv_sec = 5;
		kevent_timeout.tv_nsec = 0;

		new_events = kevent(worker.getEventQueue(), NULL, 0, &event_list[0], event_list.size(), &kevent_timeout);

		if (new_events == -1)
			throw Exception(EVENT_FAIL_KEVENT);
		
		checkReadEventList(read_event_list, cycle, worker.getEventQueue(), cgi_fd_arr, cgi_fork_list);
		checkCgiForkList(cgi_fork_list);

		for (int i = 0; i < new_events; i++) {
			cur_event = &event_list[i];
			printState(cur_event);
			if (cur_event->flags & EV_EOF)
				continue;

			// recv, send 함수에서 처리되니까 삭제해도 될 듯
			// else if (cur_event->flags & EV_ERROR) {
			// 	eventException(EVENT_SET_ERROR_FLAG, cur_event->ident);
			// 	disconnectClient(worker, cur_event->ident);
			// }
			else if (cur_event->filter == EVFILT_READ) {
				std::string*						event_type = static_cast<std::string*>(cur_event->udata);
				uintptr_t 							tmp_ident = cur_event->ident;

				if (*event_type == "listen") {
					std::cout <<"ACCEPT_NEW_CLI\n";
					// if (worker.getCurConnection() < cycle.getWorkerConnections()) //필요함
						acceptNewClient(cycle, worker.getEventQueue(), tmp_ident, server);
					// else // 연결 되지 않는 클라이언트는 어떻게 되는거지? 에러코드 바꾸기
						// eventException(EVENT_CONNECT_FULL, 0);
				}
				else if (*event_type == "client") {
					Client&			event_client = server[tmp_ident];
					// std::string&	request_msg = event_client.get_request_instance().get_request_msg();
					
					read_event_list.push_back(&event_client);
				}
				else {
					std::cout << "READ_ELSE_CUR_EVENT->IDENT: " << cur_event->ident << std::endl;
					tmp_ident = cgi_fd_arr[cur_event->ident];
					std::cout << "READ_ELSE_tmp_ident: " << tmp_ident << std::endl;
					std::cout << "BEFORE_PARSE_CGI_RESPONSE\n";
					server[tmp_ident].parse_cgi_response(server[tmp_ident].get_cgi_instance());

					//함수로 만들기
					server[tmp_ident].assemble_response();
					server[tmp_ident].get_request_instance().get_request_msg() = "";
					addEvent(worker.getEventQueue(), tmp_ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, cycle.getEventTypeClient());
					std::cout << "---------------end of assebling message--------------\n";
				}

			}
			else if (cur_event->filter == EVFILT_WRITE) {
				if (sendToClient(server[cur_event->ident]) == FALSE)
					continue;
				std::cout<< "-----------------FINISH SENDING RESPONSE MESSAGE--------------------\n";
				if (server.find (cur_event->ident) != server.end())
					server[cur_event->ident].reset_data();
			}
		}
	}
}

static void prepConnect(Cycle& cycle) {
    sockaddr_in					server_addr;
    std::list<Server>&			server_list = cycle.getServerList();
    std::list<Server>::iterator	it = server_list.begin();
    std::list<Server>::iterator	ite = server_list.end();
    std::vector<uintptr_t>&		listen_socket_list = cycle.getListenSocketList();
    uintptr_t					new_listen_socket;

    for (; it != ite; it++) {
        std::list<Server>::iterator tmp = server_list.begin();
        for (; tmp != it; tmp++)
            if (tmp->getPort() == it->getPort())
                continue;

        new_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (new_listen_socket == -1)
            throw Exception(WORK_FAIL_CREATE_SOCKET);
        listen_socket_list.push_back(new_listen_socket);

        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(it->getPort());
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // 제출 전 지우기
        int     optval = 1;
        setsockopt(new_listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(new_listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in)) == -1) {
			// listen_socket_list.clear();  // 열린 소켓들 다 닫아줘야하나?
            throw Exception(EVENT_FAIL_BIND);
		}
        if (listen(new_listen_socket, LISTEN_QUEUE_SIZE) == -1)
            throw Exception(EVENT_FAIL_LISTEN);
        if (fcntl(new_listen_socket, F_SETFL, O_NONBLOCK) == -1)
            throw Exception(EVENT_FAIL_FCNTL);
    }
}

static void addEvent(int kq, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,		\
						intptr_t data, std::string* udata) {
	kevent_t	temp;

	EV_SET(&temp, ident, filter, flags | EV_CLEAR, fflags, data, static_cast<void *>(udata));
	kevent(kq, &temp, 1, NULL, 0, NULL);
}

static void acceptNewClient(Cycle& cycle, int kq, uintptr_t listen_socket, std::map<int, Client>& server) {
	uintptr_t	client_socket;

	if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
		eventException(EVENT_FAIL_ACCEPT, 0);
		return ;
	}
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1) {
		// 소켓 닫기
		eventException(EVENT_FAIL_FCNTL, 0);
		return ;
	}
	std::cout<<"LITSENING_SOCKET: " << listen_socket << std::endl;
	std::cout<<"CLIENT_SOCKET: " << client_socket << std::endl;
	addEvent(kq, client_socket, EVFILT_READ, EV_ADD, 0, 0, cycle.getEventTypeListen());
	// worker.incCurConnection(); 필요함
}

static bool recieveFromClient(Client& client) {
	uintptr_t		client_socket = client.get_client_soket();
	std::string&	request_msg = client.get_request_instance().get_request_msg();
	char			buf[BUF_SIZE] = {0,};
	ssize_t			recieve_size;

	if ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) == 0) {
		disconnectClient(client_socket);
		eventException(EVENT_FAIL_RECV, client_socket);
		return FALSE;
	}
	std::cout << "BUFFER: " << buf << std::endl;
	buf[recieve_size] = '\0'; // 없어도 되나?
	std::string	tmp(buf, recieve_size);
	request_msg += tmp;

	std::cout << "recieve_size: " << recieve_size << ", errno: " << errno << "\n";
	std::cout << "recieve message[" << client_socket << "]:\n" << request_msg <<"\n";
	std::cout <<"---------------END_OF_RECIEVED_MESSAGE-----------------------\n";
	return TRUE;
}

static bool sendToClient(Client& client) {
	std::string response_msg = client.get_response_instance().get_response_message();
	int client_socket = client.get_client_soket();

	if (send(client_socket, response_msg.c_str(), response_msg.length() + 1, 0) == -1) {
		disconnectClient(client_socket);
		eventException(EVENT_FAIL_SEND, client_socket);
		return FALSE;
	}
	return TRUE;
}

static void disconnectClient(int client_socket) {

	std::cout << "------------------- Disconnection : client[" << client_socket << "] -------------------\n";
	close(client_socket);
	// worker.decCurConnection(); 필요함
}

static void checkCgiForkList(std::vector<Client*>& cgi_fork_list) {
	for (int i = 0; i < cgi_fork_list.size(); i++) {
		if (cgi_fork_list[i]->get_cgi() == false)
		{
			cgi_fork_list.erase(cgi_fork_list.begin() + i--);
			continue;
		}
		if (cgi_fork_list[i]->get_cgi_fork_status() == true) {
			if (cgi_fork_list[i]->get_timeout_instance().checkTimeout() == true) {
				kill(cgi_fork_list[i]->get_cgi_instance().get_pid(), SIGKILL);
				write(cgi_fork_list[i]->get_cgi_instance().get_fd(), "Status: 500\r\n\r\n", 15); //이벤트 발생 테스트해보기
				cgi_fork_list.erase(cgi_fork_list.begin() + i--);
				continue;
			}
		}
		else
		{
			Cgi::execute_cgi(cgi_fork_list[i]->get_request_instance(),	\
							cgi_fork_list[i]->get_cgi_instance());
			cgi_fork_list[i]->set_cgi_fork_status (true);
			cgi_fork_list[i]->get_timeout_instance().setSavedTime();
		}
	}
}