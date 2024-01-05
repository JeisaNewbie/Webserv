#include "core.hpp"

Worker::Worker(int id) : worker_id(id) {
	event_queue = kqueue();
	if (event_queue == -1)
		setException(WORK_CREATE_KQ_FAIL);
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == -1)
		setException(WORK_CREATE_SOCKET_FAIL);

	error_log.open("log/worker/" + std::to_string(id));
	if (error_log.is_open() == FALSE)
		setException(WORK_OPEN_FAIL);
	
	std::string	tmp = std::to_string(id);
	error_log.write(tmp.c_str(), tmp.length());
	error_log.write("\n\n", 2);
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

void createWorker(Cycle &cycle) {
	worker_array	worker_list = cycle.getWorkerList();

	for (int i = 0; i < cycle.getWorkerProcesses(); i++) {
		worker_list[i] = fork();
		if (worker_list[i] == 0) {
			try {
				prepConnect(cycle, i);
			} catch(Exception& e){
				std::cout << error_code << "\n";
				std::exit(error_code);
				// 부모에서 오류난 자식 pid랑 같이 출력하고 다시 생성
			}
		}
	}
	while (1) {
		for (int i = 0; i < cycle.getWorkerProcesses(); i++) {
			int	exit_code;
			waitpid(worker_list[i], &exit_code, WNOHANG);
			//당시 errno는 어떻게 가져오지?
			if (WEXITSTATUS(exit_code) != 0) {
				std::cerr << "pid[" << worker_list[i] << "] : " << Exception(WEXITSTATUS(exit_code)).what() << ". Restart" << "\n\n";
				// worker_list[i] = fork();
				// if (worker_list[i] == 0) {
				// 	try {
				// 		prepConnect(cycle, i);
				// 	} catch(Exception& e){
				// 		std::cerr << e.what() << std::endl;
				// 		std::exit(error_code);
				// 	}
				// }
			}
		}
	}
	//여기 중복되는 코드 정리해보기
}
