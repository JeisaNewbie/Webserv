#include "../core/core.hpp"

static void createWorker(Cycle& cycle, worker_array& worker_list, int i);

// Worker::Worker(int id) : worker_id(id), cur_connection(0) {
Worker::Worker(int id) : worker_id(id) {
	event_queue = kqueue();
	if (event_queue == -1)
		throw Exception(WORK_FAIL_CREATE_KQ);
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		throw Exception(WORK_FAIL_CREATE_SOCKET);

	error_log.open("log/" + std::to_string(id) + ".log");
	if (error_log.is_open() == FALSE)
		throw Exception(WORK_FAIL_OPEN);

	std::string	tmp = std::to_string(id);
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
	error_log.flush(); //왜 필요한거지?
}

Worker::~Worker(void) {}

pid_t Worker::getWorkerId(void) const {
	return worker_id;;
}

int	Worker::getEventQueue(void) const {
	return event_queue;
}

int	Worker::getListenSocket(void) const {
	return listen_socket;
}

std::ofstream& Worker::getErrorLog(void) {
	return error_log;
}

clients_t& Worker::getClients(void) {
	return clients;
}

kevent_t& Worker::getChangeList(void) {
	return change_list;
}

void startWorker(Cycle& cycle) {
	worker_array	worker_list = cycle.getWorkerList();

	for (int i = 0; i < cycle.getWorkerProcesses(); i++)
		createWorker(cycle, worker_list, i);
	while (1) {
		for (int i = 0; i < cycle.getWorkerProcesses(); i++) {
			int	exit_code = 0;
			waitpid(worker_list[i], &exit_code, WNOHANG);

			if (WEXITSTATUS(exit_code) != 0)
				createWorker(cycle, worker_list, i);
		}
	}
}

static void createWorker(Cycle& cycle, worker_array& worker_list, int i) {
	worker_list[i] = fork();
	if (worker_list[i] == CHILD) {
		try {
			prepConnect(cycle, i);
		} catch(Exception& e){
			std::exit(e.getCostomError());
		}
	}	
}