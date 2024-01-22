#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <iostream>

enum costom_error_type {
	PROG_INVALID_ARG_CNT = 1,
	PROG_FAIL_FUNC,

	CONF_FAIL_OPEN,
	CONF_FAIL_READ,
	CONF_DUP_DIRCTV,
	CONF_DUP_SRV_BLOCK,
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
	EVENT_CONNECT_FULL,
	EVENT_SET_ERROR_FLAG,
};

class Exception {
	public:
		Exception(int _costom_error);
		Exception(const Exception& src);
		~Exception(void);

		Exception& operator =(const Exception& src);
		
		int			getSystemError(void) const;
		int			getCostomError(void) const;
		const char* what(void) const;

	private:
		Exception(void);

		int			system_error;
		int			costom_error;
		std::string	message;
};

int		mainException(Exception& e);
void	eventException(std::ofstream& error_log, int _costom_error, uintptr_t client_fd);

#endif