#ifndef CMD_HPP
# define CMD_HPP

# include "../core/cycle.hpp"

# define CMD_NOARGS			0x00000001
# define CMD_TAKE1			0x00000010
# define CMD_TAKE2			0x00000100
# define CMD_TAKE3			0x00001000
# define CMD_TAKE4			0x00010000
# define CMD_TAKE5			0x00100000
# define CMD_TAKE6			0x01000000
# define CMD_TAKE7			0x10000000
# define CMD_TAKE_LEAST1	0x11111110

# define VALID		0
# define KILO_BYTE	1024
# define MEGA_BYTE	(KILO_BYTE * KILO_BYTE)
# define GIGA_BYTE	(KILO_BYTE * KILO_BYTE * KILO_BYTE)

# define MAX_FD		10000
# define MAX_BODY	(2LL * GIGA_BYTE)

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

void	mainWorkerConnections(Cycle& cycle, std::string tokens[]);
void	mainClientMaxBodySize(Cycle& cycle, std::string tokens[]);
void	mainRoot(Cycle& cycle, std::string tokens[]);
void	mainDefaultErrorRoot(Cycle& cycle, std::string tokens[]);

void	serverListen(Cycle& cycle, std::string tokens[]);
void	serverName(Cycle& cycle, std::string tokens[]);

void	locationRoot(Cycle& cycle, std::string tokens[]);
void	locationAllowedMethod(Cycle& cycle, std::string tokens[]);
void	locationAutoIndex(Cycle& cycle, std::string tokens[]);
void	locationIndex(Cycle& cycle, std::string tokens[]);

#endif