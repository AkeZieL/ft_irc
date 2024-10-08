#include "../../include/Parser.hpp"

void Parser::nick(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;

        if (args.empty() || args[0].empty()) {
                msg_to_client = "431 " + client->get_nickname() + " :Nickname not given\r\n"; // ERR_NONICKNAMEGIVEN
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        if(this->_serv->get_client_nickname(args[0]) != NULL) {
                msg_to_client = "433 " + client->get_nickname() + " :Nickname already used\r\n"; // ERR_NICKNAMEINUSE
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        if(client->get_connected() == false){
                client->set_nickname(args[0]);
                this->authenticated(client);
                return ;
        }
        //Si déjà connecté, il faut changer le nickname qui se trouve dans les channels
        std::vector<Channel*> client_channels = client->get_all_channel();
	std::string old_nickname = client->get_nickname();

	//Informer le client du changement de nom
        client->set_nickname(args[0]);
	msg_to_client = ":" + old_nickname + " NICK :" + client->get_nickname() + "\r\n";
	send_msg_to_client(client->get_client_fd(), msg_to_client);

	//Changer le nom dans channel
        for(std::vector<Channel*>::const_iterator it_ch = client_channels.begin(); it_ch != client_channels.end(); it_ch++) {
		(*it_ch)->set_nickname(client, args[0]);
		break ;
	}
}
