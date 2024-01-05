// #include "core.hpp"

// int openMainErrorLog(std::ofstream& error_log) {
// 	error_log.open("log/cycle");

// 	Exception e(WORK_OPEN_FAIL);

// 	if (error_log.is_open() == FALSE) {
// 		std::cerr << e.what() << std::endl;
// 		return -1;
// 	}
	
// 	error_log.write("main\n\n", 6);
// }

// int writeMainErrorLog(std::ofstream& error_log, Exception &e) {
// 	std::string	tmp = std::to_string(e.getClientFd());
// 	const char	*fd_str = tmp.c_str();
// 	const char	*errno_str = strerror(errno);

// 	error_log.write(fd_str, sizeof(*fd_str));
// 	error_log.write("\n", 1);
// 	error_log.write(e.what(), sizeof(*e.what()));
// 	if (errno != 0) {
// 		error_log.write("\n", 1);
// 		error_log.write(errno_str, sizeof(*errno_str));
// 	}
// 	error_log.write("\n\n", 2);

// 	return error_code;
// }