#include "../core/core.hpp"

static void prepConnect(Cycle& cycle, uintptr_t* socket_port_arr);
static void addEvent(Worker& worker, uintptr_t ident, int16_t filter,			\
						uint16_t flags,	uint32_t fflags,						\
						intptr_t data, void* udata);
static void acceptNewClient(Worker& worker, uintptr_t listen_socket,			\
								uint32_t port, std::map<int, Client>& server);
static bool recieveFromClient(Worker& worker, uintptr_t client_socket, intptr_t data_size);
static bool sendToClient(Worker& worker, int client_socket, Client& client);
static void disconnectClient(Worker& worker, int client_socket);

// 삭제하기


void printState(struct kevent* cur_event) {
	std::cout << "client[" << cur_event->ident << "] flags: " << cur_event->flags << ", filter: " << cur_event->filter << "\n";
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
    struct kevent tmp[2];
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;
    int new_event = kevent(kq, NULL, 0, tmp, 2, &timeout);
    std::cout << "\n\ntest: " << new_event << "\n";
    printState(&tmp[0]);
    printState(&tmp[1]);
    std::cout << "\n\n\n";
}

void startConnect(Cycle& cycle) {
    Worker					worker;
    std::vector<uintptr_t>&	listen_socket_list = cycle.getListenSocketList();
    clients_t&				clients = worker.getClients();
    kevent_t				event_list(cycle.getWorkerConnections() * 2);
    std::map<int, Client>	server;
    uintptr_t				socket_port_arr[MAX_FD] = {0,};
    uintptr_t				cgi_fd_arr[MAX_FD] = {0,};
    std::vector<Client*>	cgi_fork_list;

    prepConnect(cycle, socket_port_arr);
    std::cout << "---------------------webserver start---------------------\n";

    for (int i = 0; i < listen_socket_list.size(); i++)
        addEvent(worker, listen_socket_list[i], EVFILT_READ, EV_ADD, 0, 0, NULL);

	uint32_t		new_events;
	struct kevent*	cur_event;
	struct timespec	timeout;

	while (1) {
		timeout.tv_sec = 5;
		timeout.tv_nsec = 0;

		new_events = kevent(worker.getEventQueue(), NULL, 0, &event_list[0], event_list.size(), &timeout);

		if (new_events == -1)
			throw Exception(EVENT_FAIL_KEVENT);

		for (int i = 0; i < cgi_fork_list.size(); i++) {
			Cgi::execute_cgi(cgi_fork_list[i]->get_request_instance(),	\
								cgi_fork_list[i]->get_cgi_instance());
		}
		cgi_fork_list.clear();

		for (int i = 0; i < new_events; i++) {
			cur_event = &event_list[i];
			printState(cur_event);
			if (cur_event->flags & EV_EOF)
				continue;

			// recv, send 함수에서 처리되니까 삭제해도 될 듯
			// else if (cur_event->flags & EV_ERROR) {
			// 	eventException(worker.getErrorLog(), EVENT_SET_ERROR_FLAG, cur_event->ident);
			// 	disconnectClient(worker, cur_event->ident);
			// }
			else if (cur_event->filter == EVFILT_READ) {

				uintptr_t 							tmp_ident = cur_event->ident;
				std::vector<uintptr_t>::iterator	it = std::find(listen_socket_list.begin(),	\
																	listen_socket_list.end(),	\
																	tmp_ident);

				if (it != listen_socket_list.end()) {
					if (worker.getCurConnection() < cycle.getWorkerConnections())
						acceptNewClient(worker, *it, socket_port_arr[*it], server);
					else // 연결 되지 않는 클라이언트는 어떻게 되는거지? 에러코드 바꾸기
						eventException(worker.getErrorLog(), EVENT_CONNECT_FULL, 0);
					continue;
				}

				if (clients.find(cur_event->ident) != clients.end()) {
					if (recieveFromClient(worker, cur_event->ident, cur_event->data) == FALSE)
						continue;

					std::string&	request_msg = clients[tmp_ident];
					Client&			event_client = server[tmp_ident];

					event_client.do_parse(request_msg, cycle);
					event_client.get_response_instance().set_body("");
					// event_client.get_request_instance().check_members();
					if (event_client.get_status_code() < MOVED_PERMANENTLY && event_client.get_expect() == false)
					{
						try
						{
							if (event_client.get_cgi() == true)
							{
								event_client.set_property_for_cgi(event_client.get_request_instance());
								uintptr_t	fd = event_client.get_cgi_instance().get_fd();

								addEvent(worker, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
								cgi_fd_arr[fd] = tmp_ident;
								cgi_fork_list.push_back(&event_client);

								continue;
							}
							event_client.do_method_without_cgi(event_client.get_request_instance());
						}
						catch(int e)
						{
							event_client.set_status_code(e);
						}
					}
				}
				else {
					tmp_ident = cgi_fd_arr[cur_event->ident];
					std::cout << "BEFORE_PARSE_CGI_RESPONSE\n";
					server[tmp_ident].parse_cgi_response(server[tmp_ident].get_cgi_instance());
				}
				server[tmp_ident].assemble_response();
				clients[tmp_ident] = "";
				addEvent(worker, tmp_ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
				std::cout << "---------------end of assebling message--------------\n";

			}
			else if (cur_event->filter == EVFILT_WRITE) {
				if (sendToClient(worker, cur_event->ident, server[cur_event->ident]) == FALSE)
					continue;
				// server.erase(cur_event->ident); // 초기화
				std::cout<< "-----------------FINISH SENDING RESPONSE MESSAGE--------------------\n";
				// server[cur_event->ident].reset_data(); // 초기화
			}
		}
	}
}

static void prepConnect(Cycle& cycle, uintptr_t* socket_port_arr) {
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

		socket_port_arr[new_listen_socket] = it->getPort();

        // 제출 전 지우기
        int     optval = 1;
        setsockopt(new_listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(new_listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in)) == -1)
            throw Exception(EVENT_FAIL_BIND);
        if (listen(new_listen_socket, LISTEN_QUEUE_SIZE) == -1)
            throw Exception(EVENT_FAIL_LISTEN); // 열린 소켓들 다 닫아줘야하나?
        if (fcntl(new_listen_socket, F_SETFL, O_NONBLOCK) == -1)
            throw Exception(EVENT_FAIL_FCNTL);
    }
}

static void addEvent(Worker& worker, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,				\
						intptr_t data, void* udata) {
	struct kevent	temp;

	EV_SET(&temp, ident, filter, flags | EV_CLEAR, fflags, data, udata);
	kevent(worker.getEventQueue(), &temp, 1, NULL, 0, NULL);
}

static void acceptNewClient(Worker& worker, uintptr_t listen_socket,			\
								uint32_t port, std::map<int, Client>& server) {
	clients_t&	clients = worker.getClients();
	uintptr_t	client_socket;

	if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
		eventException(worker.getErrorLog(), EVENT_FAIL_ACCEPT, 0);
		return ;
	}
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1) {
		// 소켓 닫기
		eventException(worker.getErrorLog(), EVENT_FAIL_FCNTL, 0);
		return ;
	}

	// server[client_socket].set_port(port); 포트 번호 설정

	addEvent(worker, client_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	worker.incCurConnection();
	clients[client_socket] = "";
}

static bool recieveFromClient(Worker& worker, uintptr_t client_socket, intptr_t data_size) {
	clients_t&	clients = worker.getClients();
	char		buf[BUF_SIZE] = {0,};
	ssize_t		recieve_size;
	intptr_t	res = 0;

	while ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) > 0) {
		std::cout << "BUFFER: " << buf << std::endl;
		buf[recieve_size] = '\0';
		std::string	tmp(buf, recieve_size);
		clients[client_socket] += tmp;
		res += recieve_size;
	}
	std::cout << "recieve_size: " << recieve_size << ", errno: " << errno << "\n";
	if (recieve_size <= 0 && res != data_size) {
		disconnectClient(worker, client_socket);
		eventException(worker.getErrorLog(), EVENT_FAIL_RECV, client_socket);
		return FALSE;
	}
	else if (recieve_size == 0 && errno == EAGAIN)
		return FALSE;
	std::cout << "recieve message[" << client_socket << "]:\n" << clients[client_socket] <<"\n";
	std::cout <<"---------------END_OF_RECIEVED_MESSAGE-----------------------\n";
	return TRUE;
}

static bool sendToClient(Worker& worker, int client_socket, Client& client) {
	std::string response_msg = client.get_response_instance().get_response_message();
	if (send(client_socket, response_msg.c_str(), response_msg.length() + 1, 0) == -1) {
		disconnectClient(worker, client_socket);
		eventException(worker.getErrorLog(), EVENT_FAIL_SEND, client_socket);
		return FALSE;
	}
	return TRUE;
}

static void disconnectClient(Worker& worker, int client_socket) {
	clients_t&	clients = worker.getClients();

	std::cout << "------------------- Disconnection : client[" << client_socket << "] -------------------\n";
	close(client_socket);
	clients.erase(client_socket);
	worker.decCurConnection();
}