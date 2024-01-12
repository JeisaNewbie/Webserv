#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <iostream>

enum error_code_type {
	PROG_INVALID_ARG_CNT = 1,
	PROG_FAIL_FUNC,

	CONF_FAIL_OPEN,
	CONF_FAIL_READ,
	CONF_DUP_DIRCTV,
	CONF_INVALID_BLOCK_FORM,
	CONF_INVALID_BLOCK_LOC,
	CONF_INVALID_LOC_PATH,
	CONF_INVALID_DIRCTV,
	CONF_INVALID_DIRCTV_ARG_CNT,
	CONF_INVALID_DIRCTV_VALUE,
	CONF_INVALID_CGI,
	CONF_LACK_DIRCTV,
	CONF_FAIL_TOKENIZE,

	WORK_FAIL_OPEN,
	WORK_FAIL_CREATE_KQ,
	WORK_FAIL_CREATE_SOCKET,
	
	EVENT_FAIL_BIND,
	EVENT_FAIL_LISTEN,
	EVENT_FAIL_ACCEPT,
	EVENT_FAIL_RECV,
	EVENT_FAIL_SEND,
	EVENT_SET_ERROR_FLAG,
};

class Exception {
	public:
		Exception(int _error_code);
		Exception(const Exception& src);
		~Exception(void);

		Exception& operator =(const Exception& src);
		
		int			getErrorNum(void) const;
		const char* what(void) const;

	private:
		Exception(void);

		int			error_num;
		std::string	message;
};

void handleWorkerException(std::ofstream& error_log, int _error_code);
void handleEventException(std::ofstream& error_log, int _error_code, uintptr_t client_fd);

#endif