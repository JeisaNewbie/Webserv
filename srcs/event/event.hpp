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

		int		getEventQueue(void) const;
		int		getCurConnection(void) const;
		char*	getEventTypeListen(void);
		char*	getEventTypeClient(void);
		char*	getEventTypeCgi(void);

		void	incCurConnection(void);
		void	decCurConnection(void);
		void	addEvent(uintptr_t ident, int16_t filter,	\
						uint16_t flags,	uint32_t fflags,			\
						intptr_t data, void* udata);

	private:
		int			event_queue;
		uint32_t	cur_connection;

		char		event_type_listen[8];
		char		event_type_client[7];
		char		event_type_cgi[4];
};

void startConnect(Cycle& cycle);

#endif
