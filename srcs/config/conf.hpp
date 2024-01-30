#ifndef CONF_HPP
# define CONF_HPP

# include "cmd.hpp"

# include <fstream>
# include <sstream>

# define TOKEN_SIZE 5
# define DEFAULT_FILE "config/default.conf"
# define CMD_MAIN_MAX 4
# define CMD_SRV_MAX 2
# define CMD_LOC_MAX 4

enum conf_block_type {
	CONF_MAIN,
	CONF_SRV,
	CONF_LOC
};

class Conf {
	public:
		Conf(void);
		~Conf(void);

		void					setFile(std::string _file_name);

		std::ifstream&			getFile(void);
		const std::ifstream&	getFileConst(void) const;
		const Cmd*				getCmdListConst(int loc_type) const;
		int						getCmdMaxConst(int loc_type) const;

	private:
		Conf(const Conf& src);

		Conf& operator =(const Conf& src);

		std::ifstream		file;
		std::string			file_name;

		Cmd					main_cmd[CMD_MAIN_MAX];
		Cmd					srv_cmd[CMD_SRV_MAX];
		Cmd					loc_cmd[CMD_LOC_MAX];
};

void	setConf(Conf &conf, int argc, char* file_name);
void	parseConf(Cycle &cycle, Conf &conf);

#endif