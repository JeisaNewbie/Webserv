#include "../core/core.hpp"

static void startConnect(Cycle& cycle, Worker& worker);
static void addEvent(Worker& worker, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,				\
						intptr_t data, void* udata);
static bool acceptNewClient(Worker& worker);
static bool recieveFromClient(Worker& worker, int client_socket);
static bool sendToClient(Worker& worker, int client_socket, Client& client);
static void disconnectClient(Worker& worker, int client_socket);

void prepConnect(Cycle& cycle, int id) {
	Worker		worker(id);
	sockaddr_in	server_addr;
	int			listen_socket = worker.getListenSocket();

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	//TIME_WAIT 상태기 때문에 같은 포트 연속으로 사용 시 bind 에러. 어떻게 해결하지?
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in)) == -1)
		workerException(worker.getErrorLog(), EVENT_FAIL_BIND);
	if (listen(listen_socket, LISTEN_QUEUE_SIZE) == -1)
		workerException(worker.getErrorLog(), EVENT_FAIL_LISTEN);
	fcntl(listen_socket, F_SETFL, O_NONBLOCK);
	startConnect(cycle, worker);
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
			cur_event =& event_list[i];

			if (cur_event->flags & EV_ERROR) {
				if (cur_event->flags & EV_DELETE)
					continue;
				eventException(worker.getErrorLog(), EVENT_SET_ERROR_FLAG, cur_event->ident);
				disconnectClient(worker, cur_event->ident);
			}
			if (cur_event->filter == EVFILT_READ) {
				if (cur_event->ident == listen_socket) {
					if (acceptNewClient(worker) == FALSE)
						continue;
				}
				else if (clients.find(cur_event->ident) != clients.end()) {
					if (recieveFromClient(worker, cur_event->ident) == FALSE)
						continue;
					server[cur_event->ident].do_parse(clients[cur_event->ident], cycle);
					std::cout << "STATUS_CODE: "<< server[cur_event->ident].get_status_code() << std::endl;
					server[cur_event->ident].get_request_instance().check_members();
					// if (server[cur_event->ident].get_chunked() == true)
					// 	continue ;
					if (server[cur_event->ident].get_status_code() < BAD_REQUEST)
						server[cur_event->ident].do_method();
					server[cur_event->ident].assemble_response();
					clients[cur_event->ident] = "";
					addEvent(worker, cur_event->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
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

	EV_SET(&temp, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp);
}

static bool acceptNewClient(Worker& worker) {
	clients_t&	clients = worker.getClients();
	uintptr_t	listen_socket = worker.getListenSocket();
	uintptr_t	client_socket;

	if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
		eventException(worker.getErrorLog(), EVENT_FAIL_ACCEPT, 0);
		return FALSE;
	}
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	addEvent(worker, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	clients[client_socket] = "";
	return TRUE;
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
	if (recieve_size == 0) {
		std::cout << "------------------- worker: " << worker.getWorkerId()	\
					<< ", Disconnection : client[" << client_socket << "]\n";
		disconnectClient(worker, client_socket);
		return FALSE;
	}
	else if (recieve_size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
		disconnectClient(worker, client_socket);
		eventException(worker.getErrorLog(), EVENT_FAIL_RECV, client_socket);
		return FALSE;
	}
	std::cout << "worker: " << worker.getWorkerId() \
				<< ", size: " << clients[client_socket].length() \
				<< "\n" << clients[client_socket] << "\n";
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
}