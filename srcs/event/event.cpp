#include "../core/core.hpp"

static void startConnect(Cycle& cycle, Worker& worker);
static void addEvent(Worker& worker, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,				\
						intptr_t data, void* udata);
static void acceptNewClient(Worker& worker);
static bool recieveFromClient(Worker& worker, int client_socket);
static bool sendToClient(Worker& worker, int client_socket, Client& client);
static void disconnectClient(Worker& worker, int client_socket);

void prepConnect(Cycle& cycle) {
	Worker		worker;
	sockaddr_in	server_addr;
	int			listen_socket = worker.getListenSocket();

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in)) == -1)
		throw Exception(EVENT_FAIL_BIND);
	if (listen(listen_socket, LISTEN_QUEUE_SIZE) == -1)
		throw Exception(EVENT_FAIL_LISTEN);
	fcntl(listen_socket, F_SETFL, O_NONBLOCK);
	startConnect(cycle, worker);
}

// 삭제하기
void printState(struct kevent* cur_event) {
	std::cout << "client[" << cur_event->ident << "] flags: " << cur_event->flags << ", filter: " << cur_event->filter << "\n";
	if (cur_event->flags & EV_EOF)
		std::cout << "EOF\n";
	if (cur_event->flags & EV_DELETE)
		std::cout << "DELETE\n";
	if (cur_event->filter == EVFILT_READ)
		std::cout << "READ\n";
	if (cur_event->filter == EVFILT_WRITE)
		std::cout << "WRITE\n";
	if (errno == EAGAIN)
		std::cout << "EAGAIN\n";
	std::cout << "\n";
}

static void startConnect(Cycle& cycle, Worker& worker) {
	uintptr_t				listen_socket = worker.getListenSocket();
	clients_t&				clients = worker.getClients();
	kevent_t&				change_list = worker.getChangeList();
	kevent_t				event_list(EVENT_LIST_INIT_SIZE);
	std::map<int, Client>	server;
	std::cout << "---------------------webserver start---------------------\n";

	addEvent(worker, listen_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	uint32_t		new_events;
	struct kevent*	cur_event;
	struct timespec	timeout;
	while (1) {
		timeout.tv_sec = 10;
		timeout.tv_nsec = 0;
		//시간 나중에 수정하기

		new_events = kevent(worker.getEventQueue(),& change_list[0], change_list.size(), \
							&event_list[0], event_list.size(),& timeout);
		if (new_events > event_list.size()) {
			event_list.resize(new_events);
			kevent(worker.getEventQueue(),& change_list[0], change_list.size(), \
					&event_list[0], event_list.size(), NULL);
		}
		change_list.clear();

		for (uint32_t i = 0; i < new_events; i++) {
			cur_event = &event_list[i];
			// printState(cur_event);

			if (cur_event->flags & EV_ERROR) {
				if (cur_event->flags & EV_DELETE || errno == EAGAIN)
					continue;
				eventException(worker.getErrorLog(), EVENT_SET_ERROR_FLAG, cur_event->ident);
				disconnectClient(worker, cur_event->ident);
			}
			if (cur_event->filter == EVFILT_READ) {
				if (cur_event->ident == listen_socket) {
					if (worker.getCurConnection() < cycle.getWorkerConnections())
						acceptNewClient(worker);
					else // 연결 되지 않는 클라이언트는 어떻게 되는거지?
						eventException(worker.getErrorLog(), EVENT_FAIL_ACCEPT, 0);
				}
				else if (clients.find(cur_event->ident) != clients.end()) {
					if (recieveFromClient(worker, cur_event->ident) == FALSE)
						continue;

					std::string&	request_msg = clients[cur_event->ident];
					Client&			event_client = server[cur_event->ident];

					if (request_msg == "CGI")
						event_client.parse_cgi_response(event_client.get_cgi_instance()); //cgi process 회수 && response parse
					else {
						event_client.do_parse(request_msg, cycle);
						// event_client.get_request_instance().check_members();
						// if (event_client.get_chunked() == true)
						// 	continue ;
						if (event_client.get_status_code() < BAD_REQUEST)
						{
							event_client.do_method();
							if (event_client.get_cgi() == true)
								continue;
						}
					}
					event_client.assemble_response();
					request_msg = "";
					addEvent(worker, cur_event->ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
					std::cout << "---------------end of assebling message--------------\n";
				}
			}
			else if (cur_event->filter == EVFILT_WRITE) {
				if (sendToClient(worker, cur_event->ident, server[cur_event->ident]) == FALSE)
					continue;
				server.erase(cur_event->ident); // 초기화
				addEvent(worker, cur_event->ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
			}
		}
	}
}

static void addEvent(Worker& worker, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,				\
						intptr_t data, void* udata) {
	kevent_t&		change_list = worker.getChangeList();
	struct kevent	temp;

	EV_SET(&temp, ident, filter, flags | EV_CLEAR, fflags, data, udata);
	change_list.push_back(temp);
}

static void acceptNewClient(Worker& worker) {
	clients_t&	clients = worker.getClients();
	uintptr_t	listen_socket = worker.getListenSocket();
	uintptr_t	client_socket;

	if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
		eventException(worker.getErrorLog(), EVENT_FAIL_ACCEPT, 0);
		return ;
	}
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	addEvent(worker, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	addEvent(worker, client_socket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
	worker.incCurConnection();
	clients[client_socket] = "";
}

static bool recieveFromClient(Worker& worker, int client_socket) {
	clients_t&	clients = worker.getClients();
	char		buf[BUF_SIZE] = {0,};
	int			recieve_size;

	while ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) > 0) {
		buf[recieve_size] = '\0';
		std::string	tmp(buf, recieve_size);
		clients[client_socket] += tmp;
	}
	// std::cout << "recieve_size: " << recieve_size << ", errno: " << errno << "\n";
	if (recieve_size == -1 && errno != EAGAIN) {
		std::cout << "------------------- Disconnection : client[" << client_socket << "] -------------------\n";
		disconnectClient(worker, client_socket);
		eventException(worker.getErrorLog(), EVENT_FAIL_RECV, client_socket);
		return FALSE;
	}
	else if (recieve_size == 0 && errno == EAGAIN)
		return FALSE;
	std::cout << "recieve message: \n" << clients[client_socket] << "\n";
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

	addEvent(worker, client_socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	addEvent(worker, client_socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	close(client_socket);
	clients.erase(client_socket);
	worker.decCurConnection();
}