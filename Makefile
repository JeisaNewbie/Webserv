# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eunwolee <eunwolee@student.42seoul.kr>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/08/06 22:47:37 by eunwolee          #+#    #+#              #
#    Updated: 2024/01/19 19:58:41 by eunwolee         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

CXX = c++
# CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS_DIR = srcs
OBJS_DIR = objs

RM = rm -rf
MAKE = make

SRCS =	core/main.cpp \
		core/cycle.cpp \
		config/conf.cpp \
		config/cmd.cpp \
		block/location.cpp \
		block/server.cpp \
		worker/worker.cpp \
		event/event.cpp \
		request/Request.cpp \
		response/Response.cpp \
		method/Method.cpp \
		method/Get.cpp \
		method/Post.cpp \
		method/Delete.cpp \
		client/Client.cpp \
		cgi/Cgi.cpp \
		utils/exception.cpp \
		utils/Utils.cpp

SRCS_WITH_PATH = $(addprefix $(SRCS_DIR)/, $(SRCS))
OBJS = $(SRCS_WITH_PATH:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

RESET = \033[0m
PINK = \033[38;2;255;170;201m
GREEN = \033[38;2;179;200;144m
YELLOW = \033[38;2;255;231;155m
PECOCK = \033[38;2;115;187;201m
PURPLE = \033[38;2;228;165;255m

NUM_TASK = 11
SIZE = 0
END = 1
CHAR = ""
GAUGE = "~~~~~      "
NUM_GAUGE = 11

all:
	@echo "$(PINK)BUILD $(NAME)$(RESET)"
	@printf "$(YELLOW)$(CHAR)"
	@$(MAKE) $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $@
	$(eval END = $(shell expr $(END) - 1))
ifneq ($(END),$(NUM_GAUGE))
	$(eval END = $(shell expr $(END) + 1))
	$(eval SIZE = $(shell expr $(NUM_GAUGE) - $(END)))
	$(eval PROGRESS = $(shell echo "$(GAUGE)" | cut -c $(END)-$(shell expr $(END) + $(SIZE))))
	@printf "$(PROGRESS)"
endif
	@printf " ready!$(RESET)\n"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	$(eval SIZE = $(shell expr $(NUM_GAUGE) / $(NUM_TASK)))
	$(eval PROGRESS = $(shell echo "$(GAUGE)" | cut -c $(END)-$(shell expr $(END) + $(SIZE) - 1)))
	$(eval END = $(shell expr $(END) + $(SIZE)))
	@printf "$(PROGRESS)"
	@$(CXX) $(CXXFLAGS) -c $^ -o $@

$(OBJS_DIR):
	@mkdir ${OBJS_DIR}
	@mkdir ${OBJS_DIR}/core
	@mkdir ${OBJS_DIR}/config
	@mkdir ${OBJS_DIR}/block
	@mkdir ${OBJS_DIR}/worker
	@mkdir ${OBJS_DIR}/event
	@mkdir ${OBJS_DIR}/request
	@mkdir ${OBJS_DIR}/response
	@mkdir ${OBJS_DIR}/client
	@mkdir ${OBJS_DIR}/cgi
	@mkdir ${OBJS_DIR}/method
	@mkdir ${OBJS_DIR}/utils
	@mkdir log

clean:
	@echo "$(PECOCK)CLEANING OBJS... 🧹$(RESET)"
	@$(RM) $(OBJS_DIR)
	@$(RM) log

fclean:
	@echo "$(GREEN)CLEANING ALL... 🗑️$(RESET)"
	@$(MAKE) clean
	@$(RM) $(NAME)

re:
	@echo "$(PURPLE)RE-BUILD $(NAME)$(RESET)"
	@$(MAKE) fclean
	@$(MAKE) all

.PHONY: all clean fclean re
