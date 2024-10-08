#include "../../include/Parser.hpp"

void Parser::mode(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;
        std::string command;
	std::vector<std::string> command_arg;

        //deuxieme parsing des args
        switch(args.size()) {
                case 0:
                        msg_to_client = "461 " + client->get_nickname() + " :MODE command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                case 1:
                        msg_to_client = "461 " + client->get_nickname() + " :MODE command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                case 2:
                        command = args[1];
			break ;
                case 3:
                        command = args[1];
                        command_arg.push_back(args[2]);
			break ;
                default:
			command = args[1];
			for (size_t i = 2; i < args.size(); i++) {
				command_arg.push_back(args[i]);
			}
			break ;
        }
        //Vérifier si le channel existe
        Channel* tmp_channel = this->_serv->get_channel(args[0]);

        if(tmp_channel == NULL) {
                msg_to_client = "403 " + client->get_nickname() + " " + args[0] + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        //Verifier si le client est dans le channel
        if(tmp_channel->client_is_in_channel(client) == false) {
                msg_to_client = "441 " + client->get_nickname() + " " + args[0] + " :They aren't on that channel\r\n"; // ERR_USERNOTINCHANNEL
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        //Vérifier si le mode utiliser existe (i,t,k,o,l) ET check si changement de mode (+, -) ou consultation du mode (pas de + ou - avec mode valide)
	this->mode_check_option(client, command, command_arg, tmp_channel);
}

void Parser::mode_check_option(Client* client, std::string command, std::vector<std::string> command_arg, Channel* tmp_channel) {
	std::string msg_to_client;

	//changement des modes (ajouter ou supprimer)
        if(command[0] == '-' || command[0] == '+') {
		//Si le client n'est pas opérateur, il ne peut pas changer les modes du channel
		if (tmp_channel->client_is_operator(client) == false) {
			msg_to_client = "482 " + client->get_nickname() + " " + tmp_channel->get_channel_name() + " :You are not channel operator\r\n";
			this->send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		}
                for(size_t i = 1; i != command.size(); i++) {
                        if(command[i] != 'i' && command[i] != 't' && command[i] != 'k' && command[i] != 'o' && command[i] != 'l') {
                                msg_to_client = "472 " + client->get_nickname() + " " + command[i] + " :is unknown mode char to me for " +  tmp_channel->get_channel_name() + "\r\n"; // ERR_UNKNOWNMODE
                                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                                return ;
                        }
			if (i - 1 > 3) {
				msg_to_client = "461 " + client->get_nickname() + " :MODE can't change more than 3 option\r\n"; // ERR_NEEDMOREPARAM 
				this->send_msg_to_client(client->get_client_fd(), msg_to_client);
				return ;
			}
                }
		if (command[0] == '+') {
			tmp_channel->add_mode(client, command, command_arg);
		}
		else {
			tmp_channel->remove_mode(client, command, command_arg);
		}
        }
	//consulter les modes
        else {
                for(size_t i = 1; i != command.size(); i++) {
                        if(command[i] != 'i' && command[i] != 't' && command[i] != 'k' && command[i] != 'o' && command[i] != 'l') {
                                msg_to_client = "472 " + client->get_nickname() + " " + command[i] + " :is unknown mode char to me for " +  tmp_channel->get_channel_name() + "\r\n"; // ERR_UNKNOWNMODE
                                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                                return ;
                        }
                }
                //commande valide, répondre au client l'état du ou des modes
		tmp_channel->mode_states(client, command);
        }
}
