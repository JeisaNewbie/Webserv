#include "core.hpp"

int error_code;

int main(int argc, char **argv, char **envp) {
	(void)envp;

	Cycle			cycle;
	Conf			conf;

	try{
		setConf(conf, argc, argv[1]);
		parseConf(cycle, conf);
	} catch(Exception& e){
		std::cerr << e.what() << std::endl;
		if (errno != 0)
			std::cerr << ": " << strerror(errno) << std::endl;
		return error_code;
	}
	startWorker(cycle);
	return 0;
}
