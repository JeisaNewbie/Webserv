#include "core.hpp"

static void createWorker(Cycle &cycle, worker_array &worker_list, int i);

Worker::Worker(int id) : worker_id(id), cur_connection(0) {
	event_queue = kqueue();
	if (event_queue == -1)
		setException(WORK_CREATE_KQ_FAIL);
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		setException(WORK_CREATE_SOCKET_FAIL);

	error_log.open("log/worker/" + std::to_string(id) + ".log");
	if (error_log.is_open() == FALSE)
		setException(WORK_OPEN_FAIL);

	std::string	tmp = std::to_string(id);
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
	error_log.flush(); //왜 필요한거지?
}

Worker::Worker(const Worker& src) {
	*this = src;
}

Worker::~Worker(void) {}

Worker& Worker::operator =(const Worker& src) {
	if (this != &src) {
		worker_id = src.worker_id;
		event_queue = src.event_queue;
		listen_socket = src.listen_socket;
	}
	return (*this);
}

pid_t	Worker::getWorkerId(void) const {
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

void startWorker(Cycle &cycle) {
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

static void createWorker(Cycle &cycle, worker_array &worker_list, int i) {
	worker_list[i] = fork();
	if (worker_list[i] == CHILD) {
		try {
			prepConnect(cycle, i);
		} catch(Exception& e){
			std::exit(error_code);
		}
	}	
}