#ifndef CORE_HPP
# define CORE_HPP

# include "cycle.hpp"
# include "../config/conf.hpp"
# include "../config/cmd.hpp"
# include "../worker/worker.hpp"
# include "../event/event.hpp"
# include "../request/Request.hpp"
# include "../response/Response.hpp"
# include "../utils/exception.hpp"
# include "../utils/Status.hpp"
# include "../client/Client.hpp"

# define FALSE 0
# define TRUE 1
# define BUF_SIZE 1024

extern int error_code;

#endif