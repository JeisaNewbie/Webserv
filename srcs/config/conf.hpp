#ifndef CONF_HPP
# define CONF_HPP

# include "cmd.hpp"

# include <fstream>
# include <sstream>

# define TOKEN_SIZE 5
# define DEFAULT_FILE "default.conf"

enum conf_block_type {
	CONF_MAIN,
	CONF_SRV,
	CONF_LOC
};

class Conf {
	public:
		Conf(void);
		Conf(std::string _file_name);
		Conf(const Conf& src);
		~Conf(void);

		Conf&	operator =(const Conf& src);

		void					setFile(std::string _file_name);
		void					setCmd(void);

		std::ifstream&			getFile(void);
		const std::ifstream&	getFileConst(void) const;
		const Cmd*				getCmdListConst(int loc_type) const;
		int						getCmdMaxConst(int loc_type) const;

	private:
		std::ifstream		file;
		std::string			file_name;

		Cmd					*main_cmd;
		Cmd					*srv_cmd;
		Cmd					*loc_cmd;
		static const int	main_cmd_max;
		static const int	srv_cmd_max;
		static const int	loc_cmd_max;
};

void setConf(Conf &conf, int argc, char *file_name);
void parseConf(Cycle &cycle, Conf &conf);

#endif