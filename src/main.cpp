#include "../include/Server.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 0;
    }
    try
    {
        Server server(argv[1], argv[2]);
        server.start();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error : " << e.what() << std::endl;
        return 0;
    }
    return 1;
}
