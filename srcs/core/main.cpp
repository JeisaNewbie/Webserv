#include "core.hpp"

int main(int argc, char** argv, char** envp) {
	Cycle	cycle(const_cast<const char**>(envp));
	Conf	conf;


	cycle.getEnvp();
	try{
		setConf(conf, argc, argv[1]);
		parseConf(cycle, conf);
		startConnect(cycle);
	} catch(Exception& e){
		return mainException(e);
	}

	return 0;
}
