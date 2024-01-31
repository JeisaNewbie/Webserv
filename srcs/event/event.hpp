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

# include "../client/Client.hpp"

# define LISTEN_QUEUE_SIZE 30
# define READ_TIME_OUT 30
# define CGI_TIME_OUT 30

typedef struct kevent kevent_t;

class Event {
	public:
		Event(int event_list_size, size_t _worker_connections);
		~Event(void);


		int						getEventQueue(void) const;
		std::vector<uintptr_t>&	getListenSocketList(void);
		int						getCurConnection(void) const;
		kevent_t&				getEventOfList(int idx);
		char*					getEventTypeListen(void);
		char*					getEventTypeClient(void);
		char*					getEventTypeCgi(void);

		void		addEvent(uintptr_t ident, int16_t filter,	\
							uint16_t flags,	size_t fflags,		\
							intptr_t data, void* udata);
		size_t		pollingEvent();
		bool		checkErrorFlag(kevent_t& kevent);

		void		prepConnect(std::list<Server>& server_list);
		void		acceptNewClient(int client_socket);
		void		sendToClient(Client& client);
		int			recieveFromClient(Client& client);
		void		reclaimProcess(Client& client);
		void		disconnectClient(int client_socket);

		void		checkReadTimeout(Event& event, std::vector<Client*>& read_timeout_list);
		void		checkCgiTimeout(std::vector<Client*>& cgi_timeout_list);

	private:
		Event(const Event& src);
		Event& operator =(const Event& src);

		int						event_queue;
		std::vector<uintptr_t>	listen_socket_list;
		const size_t			worker_connections;
		size_t					cur_connection;

	    std::vector<kevent_t>	event_list;
		struct timespec			kevent_timeout;

		char					event_type_listen[8];
		char					event_type_client[7];
		char					event_type_cgi[4];
};

void startConnect(Cycle& cycle);

#endif
