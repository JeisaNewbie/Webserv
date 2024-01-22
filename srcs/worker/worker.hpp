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
		Worker(void);
		~Worker(void);

		int				getEventQueue(void) const;
		int				getCurConnection(void) const;
		clients_t&		getClients(void);
		kevent_t&		getChangeList(void);
		std::ofstream&	getErrorLog(void);

		void			incCurConnection(void);
		void			decCurConnection(void);

	private:
		Worker(const Worker& obj);
		
		Worker& operator =(const Worker& src);

		int				event_queue;
		uint32_t		cur_connection;
		clients_t		clients;
		kevent_t		change_list;
		std::ofstream	error_log;
};

#endif