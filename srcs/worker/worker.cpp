#include "../core/core.hpp"

Worker::Worker(void) : cur_connection(0) {
	event_queue = kqueue();
	if (event_queue == -1)
		throw Exception(WORK_FAIL_CREATE_KQ);
}

Worker::~Worker(void) {}

int				Worker::getEventQueue(void) const { return event_queue; }
int				Worker::getCurConnection(void) const { return cur_connection; }
clients_t&		Worker::getClients(void) { return clients; }

void			Worker::incCurConnection(void) { cur_connection++; };
void			Worker::decCurConnection(void) { cur_connection--; };
