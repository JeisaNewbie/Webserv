#ifndef EVENT_HPP
# define EVENT_HPP

# include <map>
# include <vector>
# include <cstring>
# include <fcntl.h>
# include <signal.h>
# include <iostream>
# include <sys/event.h>
# include <netinet/ip.h>

// # include <fstream>
// # include <unistd.h>
// # include <string.h>

# define LISTEN_QUEUE_SIZE 15
# define READ_TIME_OUT 30
# define CGI_TIME_OUT 30

typedef struct kevent kevent_t;

class Event {
	public:
		Event(void);
		Event(const Event& src);
		~Event(void);

		Event& operator =(const Event& src);

		int				getEventQueue(void) const;
		int				getCurConnection(void) const;

		void			incCurConnection(void);
		void			decCurConnection(void);
		std::string*	getEventTypeListen(void);
		std::string*	getEventTypeClient(void);
		std::string*	getEventTypeCgi(void);

	private:
		int			event_queue;
		uint32_t	cur_connection;

		std::string	event_type_listen;
		std::string	event_type_client;
		std::string	event_type_cgi;
};

void startConnect(Cycle& cycle);

#endif
