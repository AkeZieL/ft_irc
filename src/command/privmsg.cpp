#include "../../include/Parser.hpp"

void Parser::privmsg(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;
        std::string message;

        if(client->get_connected() == false) {
                return ;
        }
        if(args.empty() || args[0].empty() || args[1].empty())
        {
                msg_to_client = "461 " + client->get_nickname() + " :PRIVMSG command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        //reconstruire le message
        for(std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); it++) {
                if (!message.empty()){
                        message += " ";
                }
                message += *it;
        }
	//Envoyer le message
	this->send_privmsg(client, message, args);
}

void Parser::send_privmsg(Client* client, std::string message, const std::vector<std::string>& args) {
	std::string msg_to_client;

	//channel
        if (args[0][0] == '#'){
		Channel* tmp_channel = this->_serv->get_channel(args[0]);
		if (tmp_channel == NULL) {
			msg_to_client = "403 " + client->get_nickname() + " " + args[0] + " :No such channel\r\n";
                        send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
		}
		if (tmp_channel->client_is_in_channel(client) == false) {
			msg_to_client = "441 " + client->get_nickname() + " " + client->get_nickname() + " "  + tmp_channel->get_channel_name() + " :User not in channel\r\n";
                        send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
		}
		tmp_channel->send_msg_to_channel(client, message);
                return ;
        }
	//client
        else {
                Client* reciever = this->_serv->get_client_nickname(args[0]);
                if (reciever == NULL) {
                        msg_to_client = "401 " + client->get_nickname() + " " + args[0] + " :This nickname doesn't exist\r\n"; // ERR_NOSUCHNICK
                        send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                }
                msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + args[0] + " " + message + "\r\n";
                this->send_msg_to_client(reciever->get_client_fd(), msg_to_client);
                return ;
        }
}
