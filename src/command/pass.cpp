#include "../../include/Parser.hpp"

void Parser::pass(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;

        if (args.empty() || args[0].empty()) {
                msg_to_client = "461 " + client->get_nickname() + " :PASS command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        if (this->_serv->get_password() != args[0]) {
                msg_to_client = "464 " + client->get_nickname() + " :Wrong password\r\n"; // ERR_PASSWDMISMATCH
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        client->set_password(true);
        this->authenticated(client);
}
