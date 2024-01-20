#include "../core/core.hpp"

Worker::Worker(void) : cur_connection(0), error_log("log/worker.log") {
	event_queue = kqueue();
	if (event_queue == -1)
		throw Exception(WORK_FAIL_CREATE_KQ);
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		throw Exception(WORK_FAIL_CREATE_SOCKET);

	if (error_log.is_open() == FALSE)
		throw Exception(WORK_FAIL_OPEN);
}

Worker::~Worker(void) {}

int				Worker::getEventQueue(void) const { return event_queue; }
int				Worker::getListenSocket(void) const { return listen_socket; }
int				Worker::getCurConnection(void) const { return cur_connection; }
std::ofstream&	Worker::getErrorLog(void) { return error_log; }
clients_t&		Worker::getClients(void) { return clients; }
kevent_t&		Worker::getChangeList(void) { return change_list; }

void			Worker::incCurConnection(void) { cur_connection++; };
void			Worker::decCurConnection(void) { cur_connection--; };
