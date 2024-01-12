#ifndef WORKER_HPP
# define WORKER_HPP

# include <map>
# include <vector>
# include <fstream>
# include <unistd.h>
# include <string.h>
# include <sys/event.h>
# include <netinet/ip.h>

# define CHILD 0
# define PORT 80

typedef std::map<int, std::string> clients_t;
typedef std::vector<struct kevent> kevent_t;
class Worker {
	public:
		Worker(int id);
		~Worker(void);

		pid_t			getWorkerId(void) const;
		int				getEventQueue(void) const;
		int				getListenSocket(void) const;
		clients_t&		getClients(void);
		kevent_t&		getChangeList(void);
		std::ofstream&	getErrorLog(void);

	private:
		Worker(void);
		Worker(const Worker& obj);
		
		Worker& operator =(const Worker& src);

		pid_t			worker_id;
		int				event_queue;
		int				listen_socket;
		// uint32_t		cur_connection;
		clients_t		clients;
		kevent_t		change_list;
		std::ofstream	error_log;
};

void startWorker(Cycle& cycle);

#endif