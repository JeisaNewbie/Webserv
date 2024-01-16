#include "../core/core.hpp"

// Worker::Worker(int id) : worker_id(id), cur_connection(0) {
Worker::Worker(void) {
	event_queue = kqueue();
	if (event_queue == -1)
		throw Exception(WORK_FAIL_CREATE_KQ);
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		throw Exception(WORK_FAIL_CREATE_SOCKET);

	error_log.open("log/worker.log");
	if (error_log.is_open() == FALSE)
		throw Exception(WORK_FAIL_OPEN);
}

Worker::~Worker(void) {}

int				Worker::getEventQueue(void) const { return event_queue; }
int				Worker::getListenSocket(void) const { return listen_socket; }
std::ofstream&	Worker::getErrorLog(void) { return error_log; }
clients_t&		Worker::getClients(void) { return clients; }
kevent_t&		Worker::getChangeList(void) { return change_list; }
