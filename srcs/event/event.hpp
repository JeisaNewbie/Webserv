#ifndef EVENT_HPP
# define EVENT_HPP

# include <map>
# include <vector>
# include <cstring>
# include <fcntl.h>
# include <signal.h>
# include <iostream>

# define LISTEN_QUEUE_SIZE 15
# define READ_TIME_OUT 1
# define CGI_TIME_OUT 30

void startConnect(Cycle& cycle);

#endif
