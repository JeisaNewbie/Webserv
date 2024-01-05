#include "core.hpp"

static void startConnect(Cycle &cycle, Worker &worker);
static void addEvent(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, \
					uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
static void acceptNewClient(Worker &worker, int listen_socket, std::map<int, std::string> &clients,
							std::vector<struct kevent> &change_list);
static void recieveFromClient(Worker &worker, int client_socket, std::map<int, std::string> &clients);
static void disconnectClient(int client_socket, std::map<int, std::string> &clients);

void prepConnect(Cycle &cycle, int id) {
	Worker		worker(id);
	sockaddr_in	server_addr;
	int			listen_socket = worker.getListenSocket();

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_socket, reinterpret_cast<sockaddr *>(&server_addr), sizeof(sockaddr_in)) == -1)
		setException(EVENT_BIND_FAIL);
	if (listen(listen_socket, LISTEN_QUEUE_SIZE) == -1)
		setException(EVENT_LISTEN_FAIL);
	fcntl(listen_socket, F_SETFL, O_NONBLOCK);
	startConnect(cycle, worker);
}

static void startConnect(Cycle &cycle, Worker &worker) {
	uintptr_t					listen_socket = worker.getListenSocket();
	std::map<int, std::string>	clients;
	std::vector<struct kevent>	change_list, event_list(EVENT_LIST_INIT_SIZE);

	addEvent(change_list, listen_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	
	uint32_t		new_events;
	struct kevent	*cur_event;
	struct timespec	timeout;
	while (1) {
		// 여기서 무한루프 도는 중
		// 클라이언트 연결하고 데이터 보내는 것까지 간단하게 짜서 테스트해보기

		timeout.tv_sec = 1;
		timeout.tv_nsec = 0;
		//시간 나중에 수정하기

		new_events = kevent(worker.getEventQueue(), &change_list[0], change_list.size(), \
							&event_list[0], event_list.size(), &timeout);
		if (new_events > event_list.size()) {
			event_list.resize(new_events);
			kevent(worker.getEventQueue(), &change_list[0], change_list.size(), \
					&event_list[0], event_list.size(), NULL);
		}
		change_list.clear();

		for (uint32_t i = 0; i < new_events; i++) {
			cur_event = &event_list[i];

			if (cur_event->flags == EV_ERROR) {
				drivenEventException(worker.getErrorLog(), EVENT_ERROR_FLAG, cur_event->ident);
				disconnectClient(cur_event->ident, clients);
			}
			if (cur_event->filter == EVFILT_READ) {
				if (cur_event->ident == listen_socket) {
					acceptNewClient(worker, listen_socket, clients, change_list);
				}
				else if (clients.find(cur_event->ident) != clients.end()) {
					recieveFromClient(worker, cur_event->ident, clients);
					// request parsing
					(void)cycle;
					clients[cur_event->ident] = "";
				}
			}
			else if (cur_event->filter == EVFILT_WRITE) {
				// response send
				std::string	response;
				if (send(cur_event->ident, response.c_str(), response.length(), 0) == -1) {
					drivenEventException(worker.getErrorLog(), EVENT_SEND_FAIL, cur_event->ident);
					disconnectClient(cur_event->ident, clients);
				}
			}
		}
	}
}

static void addEvent(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, \
					uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent	temp;

	EV_SET(&temp, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp);
}

static void acceptNewClient(Worker &worker, int listen_socket, std::map<int, std::string> &clients,
							std::vector<struct kevent> &change_list) {
	uintptr_t	client_socket;

	if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
		drivenEventException(worker.getErrorLog(), EVENT_ACCEPT_FAIL, 0);
		return ;
	}
	
	fcntl(client_socket, F_SETFL, O_NONBLOCK);
	
	addEvent(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	addEvent(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	clients[client_socket] = "";
}

static void recieveFromClient(Worker &worker, int client_socket, std::map<int, std::string> &clients) {
	char	buf[BUF_SIZE] = {0,};
	int		recieve_size;

	while ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) != -1) {
		buf[recieve_size] = '\0';
		std::string	tmp(buf, recieve_size);
		clients[client_socket] += tmp;
	}
	if (recieve_size == -1 || errno != EAGAIN) {
		drivenEventException(worker.getErrorLog(), EVENT_RECV_FAIL, client_socket);
		disconnectClient(client_socket, clients);
	}
}

static void disconnectClient(int client_socket, std::map<int, std::string> &clients) {
	close(client_socket);
	clients.erase(client_socket);
}