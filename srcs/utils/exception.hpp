#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <iostream>

enum error_code_type {
	PROG_INVALID_ARG_CNT = 1,
	CONF_OPEN_FAIL,
	CONF_READ_FAIL,
	CONF_DIRECTIVE_OVERLAP,
	CONF_INVALID_ARG_CNT,
	CONF_INVALID_LOC,
	CONF_INVALID_LOC_TYPE,
	CONF_INVALID_FORM,
	CONF_INVALID_VALUE,
	CONF_INVALID_DIRECTIVE,
	CONF_INVALID_CGI,
	CONF_LACK_DIRECTIVE,
	CONF_TOKENIZE_FAIL,

	WORK_OPEN_FAIL,
	WORK_CREATE_KQ_FAIL,
	WORK_CREATE_SOCKET_FAIL,
	
	EVENT_BIND_FAIL,
	EVENT_LISTEN_FAIL,
	EVENT_ACCEPT_FAIL,
	EVENT_RECV_FAIL,
	EVENT_SEND_FAIL,
	EVENT_ERROR_FLAG,
};

class Exception {
	public:
		Exception(int error_code);
		//orthodox??
		const char* what() const;

	private:
		Exception(void);
		
		std::string	message;
};

void setException(int _error_code);
void handleWorkerException(std::ofstream &error_log, int _error_code);
void handleEventException(std::ofstream &error_log, int _error_code, uintptr_t client_fd);

#endif