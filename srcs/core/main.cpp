#include "core.hpp"

int error_code;

int main(int argc, char** argv, char** envp) {
	Cycle	cycle(const_cast<const char**>(envp));
	Conf	conf;

	cycle.getEnvp();
	try{
		setConf(conf, argc, argv[1]);
		parseConf(cycle, conf);
	} catch(Exception& e){
		std::cerr << "error code [" << error_code << "] : " \
					<< e.what() << std::endl;
		if (e.getErrorNum() != 0)
			std::cerr << ": " << strerror(e.getErrorNum()) << std::endl;
		return error_code;
	}
	startWorker(cycle);
	return 0;
}
