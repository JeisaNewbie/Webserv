#ifndef WORKER_HPP
# define WORKER_HPP

# include <fstream>
# include <unistd.h>
# include <string.h>
# include <sys/event.h>
# include <netinet/ip.h>

class Worker {
	public:
		Worker(int id);
		Worker(const Worker& obj);
		~Worker(void);

		Worker& operator =(const Worker& src);

		pid_t			getWorkerId(void) const;
		int				getEventQueue(void) const;
		int				getListenSocket(void) const;
		std::ofstream&	getErrorLog(void);

	private:
		Worker(void);

		pid_t			worker_id;
		int				event_queue;
		int				listen_socket;
		std::ofstream	error_log;
};

void createWorker(Cycle &cycle);

# include "event.hpp"

#endif