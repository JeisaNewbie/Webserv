#include "../core/core.hpp"

Event::Event(int event_list_size, size_t _worker_connections) : worker_connections(_worker_connections),	\
																cur_connection(0),							\
																event_type_listen("listen"),				\
																event_type_client("client"),				\
																event_type_cgi("cgi") {
	event_queue = kqueue();
	if (event_queue == -1)
		throw Exception(EVENT_FAIL_CREATE_KQ);
	event_list.resize(event_list_size);
	kevent_timeout.tv_sec = 5;
	kevent_timeout.tv_nsec = 0;
}

Event::~Event(void) {
	for (int i = 0; i < listen_socket_list.size(); i++)
		close(listen_socket_list[i]);
	close(event_queue);
}

int						Event::getEventQueue(void) const { return event_queue; }
std::vector<uintptr_t>&	Event::getListenSocketList(void) { return listen_socket_list; }
int						Event::getCurConnection(void) const { return cur_connection; }
kevent_t&				Event::getEventOfList(int idx) { return event_list[idx]; }
char*					Event::getEventTypeListen(void) { return event_type_listen; }
char*					Event::getEventTypeClient(void) { return event_type_client; }
char*					Event::getEventTypeCgi(void) { return event_type_cgi; }

void Event::addEvent(uintptr_t ident, int16_t filter,		\
						uint16_t flags,	size_t fflags,		\
						intptr_t data, void* udata) {
	kevent_t	temp;

	EV_SET(&temp, ident, filter, flags, fflags, data, udata);
	kevent(event_queue, &temp, 1, NULL, 0, NULL);
}

size_t Event::pollingEvent() {
	uint32_t	new_events;

	new_events = kevent(event_queue, NULL, 0, &event_list[0], event_list.size(), &kevent_timeout);

	if (new_events == -1)
		throw Exception(EVENT_FAIL_KEVENT);
	return new_events;
}

bool Event::checkErrorFlag(kevent_t& kevent) {
	if ((kevent.flags & EV_EOF && kevent.filter != EVFILT_PROC)	|| kevent.flags & EV_ERROR) {
		disconnectClient(kevent.ident);
		return true;
	}
	return false;
}

void Event::prepConnect(std::list<Server>& server_list) {
    sockaddr_in					server_addr;
    std::list<Server>::iterator	it = server_list.begin();
    std::list<Server>::iterator	ite = server_list.end();
    uintptr_t					new_listen_socket;

    for (; it != ite; it++) {
        std::list<Server>::iterator tmp = server_list.begin();
        for (; tmp != it; tmp++)
            if (tmp->getPort() == it->getPort())
                continue;

        new_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (new_listen_socket == -1)
            throw Exception(EVENT_FAIL_CREATE_SOCKET);
        listen_socket_list.push_back(new_listen_socket);

        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(it->getPort());
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		// socket timeout 설정
        // int     optval = 1;
        // setsockopt(new_listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(new_listen_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_in)) == -1)
            throw Exception(EVENT_FAIL_BIND);
        if (listen(new_listen_socket, LISTEN_QUEUE_SIZE) == -1)
            throw Exception(EVENT_FAIL_LISTEN);
        if (fcntl(new_listen_socket, F_SETFL, O_NONBLOCK) == -1)
            throw Exception(EVENT_FAIL_FCNTL);
        addEvent(new_listen_socket, EVFILT_READ, EV_ADD, 0, 0, &event_type_listen);
    }
}

void Event::acceptNewClient(int listen_socket) {
	size_t	client_socket;

	if (cur_connection < worker_connections) {
		if ((client_socket = accept(listen_socket, NULL, NULL)) == -1) {
			eventException(EVENT_FAIL_ACCEPT, 0);
			return ;
		}

		if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1) {
			close(client_socket);
			eventException(EVENT_FAIL_FCNTL, 0);
			return ;
		}
		addEvent(client_socket, EVFILT_READ, EV_ADD, 0, 0, &event_type_client);
		cur_connection++;
	}
	else {
		eventException(EVENT_CONNECT_FULL, 0);
	}
}

void Event::sendToClient(Client& client) {
	std::string response_msg = client.get_response_instance().get_response_message();
	int client_socket = client.get_client_soket();

	if (send(client_socket, response_msg.c_str(), response_msg.length() + 1, 0) == -1) {
		disconnectClient(client_socket);
		eventException(EVENT_FAIL_SEND, client_socket);
	}

	if (response_msg.find("Connection: close") != std::string::npos)
		disconnectClient(client_socket);
}

int Event::recieveFromClient(Client& client) {
	uintptr_t		client_socket = client.get_client_soket();
	std::string&	request_msg = client.get_request_instance().get_request_msg();
	char			buf[BUF_SIZE] = {0,};
	ssize_t			recieve_size;
	ssize_t			header_end;
	ssize_t			content_length;

	if ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) <= 0) {
		disconnectClient(client_socket);
		eventException(EVENT_FAIL_RECV, client_socket);
		return -1;
	}
	request_msg += buf;
	header_end = request_msg.find ("\r\n\r\n");

	if (header_end == std::string::npos)
	{
		client.get_timeout_instance().setSavedTime();
		return false;
	}

	if (request_msg.find("POST") == 0 && request_msg.find ("100-continue") == std::string::npos)
	{
		if (request_msg.find ("Content-Length: ") != std::string::npos)
		{
			content_length = std::strtol(request_msg.substr ((request_msg.find ("Content-Length: ") + 15), request_msg.find ("\r\n", request_msg.find ("Content-Length: ") + 15)).c_str(), NULL, 10);
			client.body_length = request_msg.size() - (header_end + 4);
			if (content_length != client.body_length)
			{
				client.get_timeout_instance().setSavedTime();
				return false;
			}
		}
		else if (request_msg.find("chunked") != std::string::npos)
		{
			if (request_msg.find ("0\r\n", header_end) == std::string::npos)
			{
				client.get_timeout_instance().setSavedTime();
				return false;
			}
			return true;
		}
	}
	return true;
}

void Event::reclaimProcess(Client& client) {
	client.parse_cgi_response(client.get_cgi_instance());

	client.assemble_response();
	client.get_request_instance().get_request_msg() = "";
	addEvent(client.get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, &event_type_client);
}

void Event::disconnectClient(int client_socket) {
	if (close(client_socket) == 0)
		cur_connection--;
}

void Event::checkReadTimeout(Event& event, std::vector<Client*>& read_timeout_list) {
	for (int i = 0; i < read_timeout_list.size(); i++) {
		if (read_timeout_list[i]->get_timeout_instance().checkTimeout(READ_TIME_OUT) == true) {
			read_timeout_list[i]->set_status_code(REQUEST_TIMEOUT);
			read_timeout_list[i]->set_read_fail(true);
			read_timeout_list[i]->assemble_response();
			event.addEvent(read_timeout_list[i]->get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, event.getEventTypeClient());
			read_timeout_list.erase(read_timeout_list.begin() + i--);
		}
	}
}

void Event::checkCgiTimeout(std::vector<Client*>& cgi_timeout_list) {
	for (int i = 0; i < cgi_timeout_list.size(); i++) {
		if (cgi_timeout_list[i]->get_cgi() == false)
		{
			cgi_timeout_list.erase(cgi_timeout_list.begin() + i--);
			continue;
		}
		if (cgi_timeout_list[i]->get_cgi_fork_status() == true) {
			if (cgi_timeout_list[i]->get_timeout_instance().checkTimeout(CGI_TIME_OUT) == true) {
				kill(cgi_timeout_list[i]->get_cgi_instance().get_pid(), SIGTERM);
				cgi_timeout_list.erase(cgi_timeout_list.begin() + i--);
				continue;
			}
		}
	}
}

void startConnect(Cycle& cycle) {
    Event					event(cycle.getWorkerConnections() * 2, cycle.getWorkerConnections());
    std::vector<uintptr_t>&	listen_socket_list = event.getListenSocketList();
    std::map<int, Client>	server;
	uint32_t				new_events;
	kevent_t*				cur_event;
	std::vector<Client*>	read_timeout_list;
	std::vector<Client*>	cgi_timeout_list;

    event.prepConnect(cycle.getServerList());

	while (1) {
		event.checkReadTimeout(event, read_timeout_list);
		event.checkCgiTimeout(cgi_timeout_list);

		new_events = event.pollingEvent();

		for (int i = 0; i < new_events; i++) {
			cur_event = &event.getEventOfList(i);

			if (event.checkErrorFlag(*cur_event) == true)
				continue;

			if (cur_event->filter == EVFILT_READ) {
				char*		event_type = static_cast<char*>(cur_event->udata);
				uintptr_t 	tmp_ident = cur_event->ident;

				if (std::strcmp(event_type, "listen") == 0) {
					event.acceptNewClient(cur_event->ident);
				}
				else if (std::strcmp(event_type, "client") == 0) {
					Client&	event_client = server[tmp_ident];
					event_client.get_request_instance().set_cycle(cycle);

					if (event_client.get_request_instance().get_request_msg() == "") {
						event_client.init_client(cur_event->ident);
						read_timeout_list.push_back(&event_client);
					}

					int	res = event.recieveFromClient(event_client);
					if (res == -1) {
						for (int i = 0; i < read_timeout_list.size(); i++) {
							if (read_timeout_list[i] == &event_client) {
								read_timeout_list.erase(read_timeout_list.begin() + i);
								break;
							}
						}
						continue;
					}
					else if (res == true) {
						event_client.do_parse(cycle);
						event_client.get_response_instance().set_body("");
						for (int i = 0; i < read_timeout_list.size(); i++) {
							if (read_timeout_list[i] == &server[tmp_ident]) {
								read_timeout_list.erase(read_timeout_list.begin() + i);
								break;
							}
						}
						for (int i = 0; i < read_timeout_list.size(); i++) {
							if (read_timeout_list[i] == &server[tmp_ident]) {
								read_timeout_list.erase(read_timeout_list.begin() + i);
								break;
							}
						}
						if (event_client.get_status_code() < MOVED_PERMANENTLY && event_client.get_expect() == false)
						{
							try
							{
								if (event_client.get_cgi() == true)
								{
									event_client.set_property_for_cgi(event_client.get_request_instance());

									pid_t	cgi_pid = Cgi::execute_cgi(event_client.get_client_soket_ptr(), event_client.get_request_instance(),	\
																		event_client.get_cgi_instance());
									if (cgi_pid != -1) {
										event.addEvent(cgi_pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, event_client.get_client_soket_ptr());
										event_client.set_cgi_fork_status (true);
										event_client.get_timeout_instance().setSavedTime();
										cgi_timeout_list.push_back(&event_client);
									}

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
					else
						continue;
					server[tmp_ident].assemble_response();
					server[tmp_ident].get_request_instance().get_request_msg() = "";
					event.addEvent(server[tmp_ident].get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, event.getEventTypeClient());
				}
			}
			else if (cur_event->filter == EVFILT_WRITE) {
				event.sendToClient(server[cur_event->ident]);
				server[cur_event->ident].reset_data();
			}
			else if (cur_event->filter == EVFILT_PROC) {
				uintptr_t	client_socket = *(static_cast<uintptr_t*>(cur_event->udata));
				event.reclaimProcess(server[client_socket]);
			}
		}
	}
}
