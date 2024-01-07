#ifndef CYCLE_HPP
# define CYCLE_HPP

# include "server.hpp"

# define WORKER_PROCESS_MAX 2
# define WORKER_CONNECTION_MAX 1024

typedef std::array<pid_t, WORKER_PROCESS_MAX> worker_array;

class Cycle {
	public:
		Cycle(void);
		Cycle(const Cycle& obj);
		~Cycle(void);

		Cycle& operator =(const Cycle& src);

		void						setWorkerProcesses(u_int32_t n);
		void						setWorkerConnections(u_int32_t n);
		void						setClientMaxBodySize(u_int32_t n);
		void						setUriLimitLength(u_int32_t n);
		void						setWorkerList(int idx, pid_t pid);

		int							getWorkerProcesses(void) const;
		int							getWorkerConnections(void) const;
		int							getClientMaxBodySize(void) const;
		size_t						getUriLimitLength(void) const;
		std::list<Server>&			getServerList(void);
		const std::list<Server>&	getServerListConst(void) const;
		worker_array				getWorkerList(void);

	private:
		int					worker_processes;
		int					worker_connections;
		size_t				client_max_body_size;
		size_t				uri_limit_length;

		std::list<Server>	server_list;
		worker_array		worker_list;
};

#endif