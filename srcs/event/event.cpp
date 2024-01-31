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

        // 제출 전 지우기
        int     optval = 1;
        setsockopt(new_listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

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

	std::cout <<"ACCEPT_NEW_CLI\n";
	std::cout << cur_connection << " " << worker_connections << "\n\n";
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
		std::cout<<"LITSENING_SOCKET: " << listen_socket << std::endl;
		std::cout<<"CLIENT_SOCKET: " << client_socket << std::endl;
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

	std::cout << "RECIEVE_FROM_CLI_CLI_SOCKET: " << client_socket << std::endl;
	if ((recieve_size = recv(client_socket, buf, BUF_SIZE - 1, 0)) <= 0) {
		disconnectClient(client_socket);
		eventException(EVENT_FAIL_RECV, client_socket);
		return -1;
	}
	std::cout << "BUFFER: " << buf << std::endl;
	request_msg += buf;
	header_end = request_msg.find ("\r\n\r\n");
	std::cout <<"-ZERO\n";

	if (header_end == std::string::npos)
	{
		client.get_timeout_instance().setSavedTime();
		std::cout <<"--ZERO\n";
		return false;
	}

	std::cout <<"ZERO\n";
	if (request_msg.find("POST") == 0 && request_msg.find ("100-continue") == std::string::npos)
	{
		std::cout <<"ONE\n";
		if (request_msg.find ("Content-Length: ") != std::string::npos)
		{
			std::cout <<"TWO\n";
			content_length = std::strtol(request_msg.substr ((request_msg.find ("Content-Length: ") + 15), request_msg.find ("\r\n", request_msg.find ("Content-Length: ") + 15)).c_str(), NULL, 10);
			client.body_length = request_msg.size() - (header_end + 4);
			std::cout << "BODY_LENGTH: " << client.body_length << std::endl;
			std::cout << "NEW_REQUEST_MSG: \n" << request_msg << std::endl;
			std::cout <<"TWO_TWO\n";
			if (content_length != client.body_length)
			{
				client.get_timeout_instance().setSavedTime();
				return false;
			}
		}
		else if (request_msg.find("chunked") != std::string::npos)
		{
			std::cout <<"THREE\n";
			if (request_msg.find ("0\r\n", header_end) == std::string::npos)
			{
				std::cout <<"FOUR\n";
				client.get_timeout_instance().setSavedTime();
				return false;
			}
			std::cout << "CHUNKED_DONE: " << request_msg.find("0\r\n") << std::endl;
			return true;
		}
	}

	std::cout << "recieve_size: " << recieve_size << ", errno: " << errno << "\n";
	std::cout << "recieve message[" << client_socket << "]:\n" << request_msg <<"\n";
	std::cout <<"---------------END_OF_RECIEVED_MESSAGE-----------------------\n";
	return true;
}

void Event::reclaimProcess(Client& client) {
	client.parse_cgi_response(client.get_cgi_instance());

	std::cout << "START_ASSEMBLE_CGI_RESPONSE\n";
	client.assemble_response();
	client.get_request_instance().get_request_msg() = "";
	addEvent(client.get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, &event_type_client);
	std::cout << "---------------end of assebling message--------------\n";
}

void Event::disconnectClient(int client_socket) {
	std::cout << "------------------- Disconnection : client[" << client_socket << "] -------------------\n";
	close(client_socket);
	cur_connection--;
}

void Event::checkReadTimeout(Event& event, std::vector<Client*>& read_timeout_list) {
	for (int i = 0; i < read_timeout_list.size(); i++) {
		if (read_timeout_list[i]->get_timeout_instance().checkTimeout(READ_TIME_OUT) == true) {
			read_timeout_list[i]->set_status_code(REQUEST_TIMEOUT);
			std::cout << "CHECK_READ_TIMEOUT_ASSEMBLE_RESPONSE\n";
			read_timeout_list[i]->set_read_fail(true);
			read_timeout_list[i]->assemble_response();
			event.addEvent(read_timeout_list[i]->get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, event.getEventTypeClient());
			read_timeout_list.erase(read_timeout_list.begin() + i--);
		}
	}
}

void Event::checkCgiTimeout(std::vector<Client*>& cgi_timeout_list) {
	for (int i = 0; i < cgi_timeout_list.size(); i++) {
		std::cout << "CGI_SOCKET: " << cgi_timeout_list[i]->get_client_soket() <<  ", CGI_FORK_STATUS: " << cgi_timeout_list[i]->get_cgi_fork_status() \
		 << ", CGI_STATUS: " << cgi_timeout_list[i]->get_cgi() << "\n";
		if (cgi_timeout_list[i]->get_cgi() == false)
		{
			cgi_timeout_list.erase(cgi_timeout_list.begin() + i--);
			continue;
		}
		if (cgi_timeout_list[i]->get_cgi_fork_status() == true) {
			if (cgi_timeout_list[i]->get_timeout_instance().checkTimeout(CGI_TIME_OUT) == true) {
				std::cout << "CGI_SCRIPT_TIMEOUT\n";
				kill(cgi_timeout_list[i]->get_cgi_instance().get_pid(), SIGTERM);
				std::cout << "\n\nCGI_KILL\n\n";
				cgi_timeout_list.erase(cgi_timeout_list.begin() + i--);
				continue;
			}
		}
	}
}

// 삭제하기
void printState(kevent_t* cur_event) {
	std::cout << "client[" << cur_event->ident << "]: " << cur_event->data << "\n";
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
	if (cur_event->filter == EVFILT_PROC)
		std::cout << "PROC\n";
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

void startConnect(Cycle& cycle) {
    Event					event(cycle.getWorkerConnections() * 2, cycle.getWorkerConnections());
    std::vector<uintptr_t>&	listen_socket_list = event.getListenSocketList();
    std::map<int, Client>	server;
	uint32_t				new_events;
	kevent_t*				cur_event;
	std::vector<Client*>	read_timeout_list;
	std::vector<Client*>	cgi_timeout_list;

    event.prepConnect(cycle.getServerList());
    std::cout << "---------------------webserver start---------------------\n";

	while (1) {
		std::cout << "CHECK_READ_LIST\n";
		event.checkReadTimeout(event, read_timeout_list);
		std::cout << "CHECK_FORK_LIST\n";
		event.checkCgiTimeout(cgi_timeout_list);

		std::cout << "KEVENT\n";
		new_events = event.pollingEvent();
		std::cout << "NEW_EVENT: " << new_events << "\n";

		for (int i = 0; i < new_events; i++) {
			cur_event = &event.getEventOfList(i);
			printState(cur_event);

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
					std::cout << "RES: " << res << std::endl;
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
									std::cout << "CGI_BODY: " << event_client.get_request_instance().get_message_body () << std::endl;
									event_client.set_property_for_cgi(event_client.get_request_instance());

									std::cout << "\n\nEXECUTE_CGI\n\n\n";
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
					{
						std::cout << "RES_FAIL\n";
						continue;
					}
					std::cout << "START_ASSEMBLE_RESPONSE\n";
					server[tmp_ident].assemble_response();
					server[tmp_ident].get_request_instance().get_request_msg() = "";
					event.addEvent(server[tmp_ident].get_client_soket(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, event.getEventTypeClient());
					std::cout << "---------------end of assebling message--------------\n";
				}
			}
			else if (cur_event->filter == EVFILT_WRITE) {
				event.sendToClient(server[cur_event->ident]);
				server[cur_event->ident].reset_data();
				std::cout<< "-----------------FINISH SENDING RESPONSE MESSAGE--------------------\n";
			}
			else if (cur_event->filter == EVFILT_PROC) {
				uintptr_t	client_socket = *(static_cast<uintptr_t*>(cur_event->udata));
				std::cout << "BEFORE_PARSE_CGI_RESPONSE\n";
				std::cout << "CLIENT_SOCKET: " << client_socket << "\n";
				event.reclaimProcess(server[client_socket]);
			}
		}
	}
}
