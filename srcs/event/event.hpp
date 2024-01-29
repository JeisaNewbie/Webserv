#ifndef EVENT_HPP
# define EVENT_HPP

# include <map>
# include <ctime>
# include <vector>
# include <cstring>
# include <fcntl.h>
# include <signal.h>
# include <iostream>

# define LISTEN_QUEUE_SIZE 15
# define TIME_OUT 30

void startConnect(Cycle& cycle);

#endif

