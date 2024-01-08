#ifndef CMD_HPP
# define CMD_HPP

# include "cycle.hpp"

enum cmd_args {
	CMD_NOARGS,
	CMD_TAKE1,
	CMD_TAKE2
};

# define KILO_BYTE 1024

typedef void (*handler_t)(Cycle&, std::string[]);

class Cmd {
	public:
		Cmd(void);
		Cmd(std::string _name, int _arg_cnt, handler_t _handler);
		Cmd(const Cmd& obj);
		~Cmd(void);

		Cmd& operator =(const Cmd& src);

		const std::string&	getName(void) const;
		int					getArgCnt(void) const;
		const handler_t&	getHandler(void) const;

	private:
		std::string	name;
		int			arg_cnt;
		handler_t	handler;
};

void mainWorkerProcesses(Cycle& cycle, std::string tokens[]);
void mainWorkerConnections(Cycle& cycle, std::string tokens[]);
void mainClientMaxBodySize(Cycle& cycle, std::string tokens[]);
void mainUriLimitLength(Cycle& cycle, std::string tokens[]);
void mainRoot(Cycle& cycle, std::string tokens[]);

void serverListen(Cycle& cycle, std::string tokens[]);
void serverName(Cycle& cycle, std::string tokens[]);

void locationRoot(Cycle& cycle, std::string tokens[]);

#endif