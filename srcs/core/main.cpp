#include "core.hpp"

int main(int argc, char** argv, char** envp) {
	Cycle	cycle(const_cast<const char**>(envp));
	Conf	conf;

	cycle.getEnvp();
	try{
		setConf(conf, argc, argv[1]);
		parseConf(cycle, conf);
	} catch(Exception& e){
		return mainException(e);
	}
	startWorker(cycle);
	return 0;
}
