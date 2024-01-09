#ifndef CYCLE_HPP
# define CYCLE_HPP

# include "../block/server.hpp"

# include <list>
# include <iostream>

# define WORKER_PROCESS_MAX 2
# define WORKER_CONNECTION_MAX 1024

typedef std::array<pid_t, WORKER_PROCESS_MAX> worker_array;

class Cycle {
	public:
		Cycle(const char **_envp);
		Cycle(const Cycle& obj);
		~Cycle(void);

		Cycle& operator =(const Cycle& src);

		void						setUseCgi(bool state);
		void						setWorkerProcesses(u_int32_t n);
		void						setWorkerConnections(u_int32_t n);
		void						setClientMaxBodySize(u_int32_t n);
		void						setUriLimitLength(u_int32_t n);
		void						setMainRoot(std::string _path);
		void						setWorkerList(int idx, pid_t pid);

		const char**				getEnvp(void) const;
		bool						getUseCgi(void) const;
		int							getWorkerProcesses(void) const;
		int							getWorkerConnections(void) const;
		int							getClientMaxBodySize(void) const;
		int							getUriLimitLength(void) const;
		const std::string&			getMainRoot(void) const;
		std::list<Server>&			getServerList(void);
		const std::list<Server>&	getServerListConst(void) const;
		worker_array				getWorkerList(void);

	private:
		Cycle(void);

		const char			**envp;
		bool				use_cgi;

		int					worker_processes;
		int					worker_connections;
		size_t				client_max_body_size;
		size_t				uri_limit_length;
		std::string			main_root;

		std::list<Server>	server_list;
		worker_array		worker_list;
};

#endif