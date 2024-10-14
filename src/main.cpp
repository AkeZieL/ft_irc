#include "../include/Server.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 0;
    }
    try
    {
        signal(SIGINT, Server::signal_handler);
        signal(SIGQUIT, Server::signal_handler);
        Server server(argv[1], argv[2]);
        server.start();
    }
    catch (std::exception &e)
    {
        std::cerr << "\nError : " << e.what() << std::endl;
        return 0;
    }
    return 1;
}
