#include "../../include/Parser.hpp"

void Parser::user(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;

        if (args.size() < 4) {
                msg_to_client = "461 " + client->get_nickname() + " :USER command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        client->set_username(args[0]);
        client->set_realname(args[3]);
        this->authenticated(client);
}
