#include "core.hpp"

int error_code;

int main(int argc, char **argv, char **envp) {
	(void)envp;

	Cycle			cycle;
	Conf			conf;
	std::ofstream	error_log;

	// if (openMainErrorLog(error_log) == -1)
	// 	return -1;

	try{
		setConf(conf, argc, argv[1]);
		parseConf(cycle, conf);
		std::cout<< "hello world\n";
	} catch(Exception& e){
		std::cerr << e.what() << std::endl;
		if (errno != 0)
			std::cerr << ": " << strerror(errno) << std::endl;
		return error_code;
	}
	createWorker(cycle);
	return 0;
}
