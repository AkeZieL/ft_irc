NAME = ircserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRCS = src/main.cpp src/Server.cpp src/Client.cpp src/Parser.cpp src/Channel.cpp

COMMAND_SRCS = src/command/nick.cpp src/command/user.cpp src/command/pass.cpp src/command/privmsg.cpp src/command/part.cpp src/command/join.cpp src/command/mode.cpp src/command/topic.cpp src/command/list.cpp src/command/name.cpp src/command/whois.cpp src/command/ping.cpp src/command/pong.cpp src/command/quit.cpp src/command/kick.cpp

OBJS = $(SRCS:.cpp=.o)

COMMAND_OBJS = $(COMMAND_SRCS:.cpp=.o)

all : $(NAME)

%.o: %.cpp
	g++ $(CFLAGS) -I./ -c $< -o $@

$(NAME) : $(OBJS) $(COMMAND_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(COMMAND_OBJS) -g -o $(NAME)

clean :
	rm -rf src/*.o
	rm -rf src/command/*.o

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re
