#include "../../include/Parser.hpp"

void Parser::part(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;
	std::string msg_to_send;
        std::vector<std::string> channel;
        std::vector<std::string> message;

        //deuxieme parsing pour les args
        switch(args.size()) {
                case 0:
                        msg_to_client = "461 " + client->get_nickname() + " :PART command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                case 1:
                        channel = parse_join_args(args[0]);
			msg_to_send = "";
                        break ;
                default:
                        channel = parse_join_args(args[0]);
			//récupérer le message
                        for(std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); it++) {
                                message.push_back(*it);
                        }
			//reconstruire le message
                        for(std::vector<std::string>::iterator it = message.begin(); it != message.end(); it++) {
                                if(!msg_to_send.empty()) {
                                        msg_to_send.append(" ");
                                }
                                msg_to_send.append(*it);
                        }
                        break ;
        };
        //quitter channel et envoyé message si dispo
	this->part_channel(client, channel, msg_to_send);
}

void Parser::part_channel(Client* client, std::vector<std::string> channel, std::string message) {
	std::string msg_to_client;
	Channel* tmp_channel = NULL;

	for(size_t i = 0; i != channel.size(); i++) {
                tmp_channel = this->_serv->get_channel(channel[i]); //Ce n'est pas une copie, si je modifie tmp_channel, je modifie aussi le channel du serv
                //Vérifier si le nom du channel est valide + si le channel existe
                if(channel[i][0] != '#' || tmp_channel == NULL){
                        msg_to_client = "403 " + client->get_nickname() + " " + channel[i] + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        continue ;
                }
                //Verifie si le client est dans le channel
                if(tmp_channel->client_is_in_channel(client) == false) {
                        msg_to_client = "442 " + client->get_nickname() + " " + channel[i] + " :You're not on that channel\r\n"; // ERR_NOTONCHANNEL
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                }
                //Quitter le channel et envoyer le message si il y en a un
		client->part_channel(client, tmp_channel, message);
        }
}
