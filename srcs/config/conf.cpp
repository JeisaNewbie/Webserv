#include "../core/core.hpp"

static void	parseMain(Cycle& cycle, Conf& conf, std::ifstream& file);
static void	parseServer(Cycle& cycle, Conf& conf, std::ifstream& file);
static void	parseLocation(Cycle& cycle, Conf& conf, std::ifstream& file,	\
							const std::string& location_path);
static void	callCmd(Cycle& cycle, Conf& conf, int location,					\
						std::string* tokens, int token_cnt);
static int	tokenizer(char* str, std::string* tokens);
static int	checkConfLocation(std::string str[]);
static void	checkGetlineError(std::ifstream& file);
static void checkServerDuplication(std::list<Server>& server_list);
static void checkLocationDuplication(std::list<Location>& location_list);
static void checkMaxDomainSize(Cycle& cycle);
static int	setTakeArgCnt(int cnt);

Conf::Conf(void) {
	main_cmd[0] = Cmd("worker_connections",					\
						CMD_TAKE1,							\
						mainWorkerConnections);
	main_cmd[1] = Cmd("client_max_body_size",				\
						CMD_TAKE1,							\
						mainClientMaxBodySize);
	main_cmd[2] = Cmd("root",								\
						CMD_TAKE1,							\
						mainRoot);
	main_cmd[3] = Cmd("default_error_root",					\
						CMD_TAKE1,							\
						mainDefaultErrorRoot);

	srv_cmd[0] = Cmd("listen",								\
						CMD_TAKE1,							\
						serverListen);
	srv_cmd[1] = Cmd("server_name", 						\
						CMD_TAKE1,							\
						serverName);

	loc_cmd[0] = Cmd("root",								\
						CMD_TAKE1,							\
						locationRoot);
	loc_cmd[1] = Cmd("allowed_method",						\
						CMD_TAKE1 | CMD_TAKE2 | CMD_TAKE3,	\
						locationAllowedMethod);
	loc_cmd[2] = Cmd("autoindex",							\
						CMD_TAKE1,							\
						locationAutoIndex);
	loc_cmd[3] = Cmd("index",								\
						CMD_TAKE_LEAST1,					\
						locationIndex);
}

Conf::~Conf(void) {}

void Conf::setFile(std::string _name) {
	file_name = _name;
	file.open(file_name);
	if (file.is_open() == FALSE)
		throw Exception(CONF_FAIL_OPEN, file_name);
}

std::ifstream&			Conf::getFile(void) { return file; }
const std::ifstream&	Conf::getFileConst(void) const { return file; }

const Cmd* Conf::getCmdListConst(int loc_type) const {
	if (loc_type == CONF_MAIN)
		return main_cmd;
	if (loc_type == CONF_SRV)
		return srv_cmd;
	if (loc_type == CONF_LOC)
		return loc_cmd;
	return NULL;
}

int Conf::getCmdMaxConst(int loc_type) const {
	if (loc_type == CONF_MAIN)
		return CMD_MAIN_MAX;
	if (loc_type == CONF_SRV)
		return CMD_SRV_MAX;
	if (loc_type == CONF_LOC)
		return CMD_LOC_MAX;
	return -1;
}

void setConf(Conf& conf, int argc, char* file_name) {
	if (argc != 1 && argc != 2)
		throw Exception(PROG_INVALID_ARG_CNT);
	if (argc == 1)
		conf.setFile(DEFAULT_FILE);
	else
		conf.setFile(file_name);
}

void parseConf(Cycle& cycle, Conf& conf) {
	char			buf[BUF_SIZE];
	std::string		tokens[TOKEN_SIZE];
	int				token_cnt;
	std::ifstream&	file = conf.getFile();

	while (file.getline(buf, sizeof(buf))) {
		if (buf[0] == '\0')
			continue;

		for (int i = 0; i < TOKEN_SIZE; i++)
			tokens[i] = '\0';

		token_cnt = tokenizer(buf, tokens);
		if (token_cnt != 2 || tokens[1] != "{")
			throw Exception(CONF_INVALID_BLOCK_FORM, tokens[1]);

		if (checkConfLocation(tokens) == CONF_MAIN)
			parseMain(cycle, conf, file);
		else if (checkConfLocation(tokens) == CONF_SRV)
			parseServer(cycle, conf, file);
	}
	checkGetlineError(file);

	if (cycle.getWorkerConnections() == 0		\
		|| cycle.getClientMaxBodySize() == 0	\
		|| cycle.getMainRoot() == ""			\
		|| cycle.getDefaultErrorRoot() == ""	\
		|| cycle.getServerList().size() == 0)
		throw Exception(CONF_LACK_DIRCTV, "Main block");

	checkMaxDomainSize(cycle);
}

static void parseMain(Cycle& cycle, Conf& conf, std::ifstream& file) {
	char			buf[BUF_SIZE];
	std::string		tokens[TOKEN_SIZE];
	int				token_cnt;
	std::string		str_buf;

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "}")
			break;

		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][1];
		callCmd(cycle, conf, CONF_MAIN, tokens, setTakeArgCnt(token_cnt));
	}
	checkGetlineError(file);
}

static void parseServer(Cycle& cycle, Conf& conf, std::ifstream& file) {
	char				buf[BUF_SIZE];
	std::string			tokens[TOKEN_SIZE];
	int					token_cnt;
	std::string			str_buf;
	std::list<Server>&	server_list = cycle.getServerList();

	server_list.push_back(Server());

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "}")
			break;

		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][1];

		if (tokens[0] == "location") {
			if (token_cnt != 3 || tokens[2] != "{")
				throw Exception(CONF_INVALID_BLOCK_FORM, tokens[2]);
			parseLocation(cycle, conf, file, tokens[1]);
			continue;
		}
		callCmd(cycle, conf, CONF_SRV, tokens, setTakeArgCnt(token_cnt));
	}
	checkGetlineError(file);
	checkServerDuplication(server_list);

	if (server_list.back().getPort() == 0		\
		|| server_list.back().getDomain() == ""	\
		|| server_list.back().getLocationList().size() == 0)
		throw Exception(CONF_LACK_DIRCTV, "Server block");
}

static void parseLocation(Cycle& cycle, Conf& conf, std::ifstream& file,	\
							const std::string& location_path) {
	char					buf[BUF_SIZE];
	std::string				tokens[TOKEN_SIZE];
	int						token_cnt;
	std::string				str_buf;
	std::list<Location>&	location_list = cycle.getServerList().back().getLocationList();

	checkLocationDuplication(location_list);
	location_list.push_back(Location(location_path));

	while (file.getline(buf, sizeof(buf))) {
		str_buf = static_cast<std::string>(buf);

		if (str_buf.length() == 0)
			continue;
		if (str_buf == "\t}")
			break;

		token_cnt = tokenizer(buf, tokens);
		tokens[0] = &tokens[0][2];
		callCmd(cycle, conf, CONF_LOC, tokens, setTakeArgCnt(token_cnt));
	}
	checkGetlineError(file);

	if (location_list.back().getSubRoot() == "")
		throw Exception(CONF_LACK_DIRCTV, "Location block");

	if (location_list.back().getAllowedMethod() == 0)
		location_list.back().setAllowedMethod(				\
			METHOD_GET | METHOD_POST | METHOD_DELETE);
	if (location_list.back().getAutoIndex() == -1)
		location_list.back().setAutoIndex(FALSE);
}

static void callCmd(Cycle& cycle, Conf& conf, int location, \
						std::string* tokens, int token_cnt) {
	handler_t	handler;
	int 		idx;
	const Cmd*	cmd = conf.getCmdListConst(location);
	int			cmd_max = conf.getCmdMaxConst(location);

	for (idx = 0; idx < cmd_max; idx++) {
		if (cmd[idx].getName() == tokens[0]) {
			if ((cmd[idx].getArgCnt() & token_cnt) == 0)
				throw Exception(CONF_INVALID_DIRCTV_ARG_CNT, tokens[0]);
			handler = cmd[idx].getHandler();
			try {
				handler(cycle, tokens);
			} catch (const std::invalid_argument& e) {
				throw Exception(PROG_FAIL_FUNC);
			} catch (const std::out_of_range& e) {
				throw Exception(PROG_FAIL_FUNC);
			}
			break;
		}
	}
	if (idx == cmd_max)
		throw Exception(CONF_INVALID_DIRCTV, tokens[0]);
}

static int tokenizer(char* str, std::string* tokens) {
	std::istringstream	istr(str);
	std::string			token;
	int					idx = 0;

	while (getline(istr, token, ' '))
		if (token.empty() == FALSE)
			tokens[idx++] = token;

	if (istr.eof() == FALSE)
		throw Exception(CONF_FAIL_TOKENIZE);
	return idx;
}

static int checkConfLocation(std::string str[]) {
	if (str[0] == "main")
		return CONF_MAIN;
	if (str[0] == "server")
		return CONF_SRV;
	throw Exception(CONF_INVALID_BLOCK_LOC, str[0]);
	return 0;
}

static void checkGetlineError(std::ifstream& file) {
	if (file.eof() != TRUE \
		&& (file.fail() == TRUE || file.bad() == TRUE))
		throw Exception(CONF_FAIL_READ);
}

static void checkServerDuplication(std::list<Server>& server_list) {
	std::list<Server>::iterator			it = server_list.begin();
	std::list<Server>::reverse_iterator	ite = server_list.rbegin();

	for (; &(*it) != &(*ite); it++) {
		if (ite->getPort() == it->getPort() && ite->getDomain() == it->getDomain())
			throw Exception(CONF_DUP_SRV_BLOCK, it->getDomain() + ":" + to_string(it->getPort()));
	}
}

static void checkLocationDuplication(std::list<Location>& location_list) {
	std::list<Location>::iterator			it = location_list.begin();
	std::list<Location>::reverse_iterator	ite = location_list.rbegin();

	for (; &(*it) != &(*ite); it++) {
		if (ite->getLocationPath() == it->getLocationPath())
			throw Exception(CONF_DUP_LOC_BLOCK, it->getLocationPath());
	}
}

static void checkMaxDomainSize(Cycle& cycle) {
	size_t						tmp, max_len = 0;
	std::list<Server>&			server_list = cycle.getServerList();
	std::list<Server>::iterator	it = server_list.begin();
	std::list<Server>::iterator	ite = server_list.end();

	for (; it != ite; it++) {
		tmp = it->getDomain().length();
		max_len = tmp > max_len ? tmp : max_len;
	}
	cycle.setUriLimitLength(max_len + 50);
}

static int setTakeArgCnt(int cnt) {
	cnt--;
	if (cnt == 0)
		return CMD_NOARGS;
	if (cnt == 1)
		return CMD_TAKE1;
	if (cnt == 2)
		return CMD_TAKE2;
	if (cnt == 3)
		return CMD_TAKE3;
	if (cnt == 4)
		return CMD_TAKE4;
	if (cnt == 5)
		return CMD_TAKE5;
	if (cnt == 6)
		return CMD_TAKE6;
	if (cnt == 7)
		return CMD_TAKE7;
	throw Exception(CONF_INVALID_DIRCTV_ARG_CNT);
	return 0;
}