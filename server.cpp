#include "srcs/core/core.hpp"

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

void addEvent(int kq, uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,				\
						intptr_t data, void* udata) {
	struct kevent	temp;

	EV_SET(&temp, ident, filter, flags | EV_CLEAR, fflags, data, udata);
	kevent(kq, &temp, 1, NULL, 0, NULL);
}

int acceptNewClient(int kq, uintptr_t listen_socket) {
	uintptr_t	client_socket;

	client_socket = accept(listen_socket, NULL, NULL);
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	addEvent(kq, client_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	return client_socket;
}

bool recieveFromClient(uintptr_t client_socket, intptr_t data_size) {
	char		buf[BUF_SIZE] = {0,};
	std::string	recieve_msg;
	ssize_t		recieve_size;
	intptr_t	res = 0;

	while ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) > 0) {
		buf[recieve_size] = '\0';
		std::string	tmp(buf, recieve_size);
		recieve_msg += tmp;
		res += recieve_size;
	}
	std::cout << "recieve_size: " << recieve_size << ", errno: " << errno << "\n";
	if (errno == ECONNRESET) {
		std::cout << "------------------- Disconnection : client[" << client_socket << "] -------------------\n";
		close(client_socket);
		return FALSE;
	}
	if (res != data_size)
		return FALSE;
	std::cout << "recieve message: \n" << recieve_msg << "\n\n";
	return TRUE;
}

bool sendToClient(int client_socket) {
	std::string response_msg = "server";
	if (send(client_socket, response_msg.c_str(), response_msg.length() + 1, 0) == -1) {
		close(client_socket);
		return FALSE;
	}
	return TRUE;
}

int main() {
	int kq = kqueue();

    sockaddr_in server_addr;
	uintptr_t	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in));
	listen(listen_socket, LISTEN_QUEUE_SIZE);
	fcntl(listen_socket, F_SETFL, O_NONBLOCK);

    addEvent(kq, listen_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);

    std::vector<struct kevent>	event_list(8);
	struct timespec	timeout;
	timeout.tv_sec = 5;
	timeout.tv_nsec = 0;

	uintptr_t	client_socket;

	while (1) {
		int new_events = kevent(kq, NULL, 0, &event_list[0], 8, &timeout);
		for (int i = 0; i < new_events; i++) {
			struct kevent* cur_event = &event_list[i];
			std::cout << "start:\n";
			printState(cur_event);

			if (cur_event->ident == listen_socket) {
				client_socket = acceptNewClient(kq, listen_socket);
			    addEvent(kq, client_socket, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
			}
			else if (cur_event->ident == client_socket) {
				if (cur_event->filter == EVFILT_READ) {
					if (recieveFromClient(client_socket, cur_event->data) == TRUE)
				    	addEvent(kq, client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
				}
				else if (cur_event->filter == EVFILT_WRITE) {
					if (sendToClient(client_socket) == TRUE)
				    	addEvent(kq, client_socket, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
				}
			}
			std::cout << "end:\n";
			printState(cur_event);
		}
	}

	return 0;
}
