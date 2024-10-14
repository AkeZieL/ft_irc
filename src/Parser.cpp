#include "../include/Parser.hpp"

Parser::Parser(Server* server) : _serv(server) {
	this->_commands["NICK"] = &Parser::nick;
	this->_commands["USER"] = &Parser::user;
	this->_commands["PASS"] = &Parser::pass;
	this->_commands["PRIVMSG"] = &Parser::privmsg;
	this->_commands["JOIN"] = &Parser::join;
	this->_commands["PART"] = &Parser::part;
	this->_commands["MODE"] = &Parser::mode;
	this->_commands["TOPIC"] = &Parser::topic;
	this->_commands["LIST"] = &Parser::list;
	this->_commands["NAMES"] = &Parser::names;
	this->_commands["PING"] = &Parser::ping;
	this->_commands["PONG"] = &Parser::pong;
	this->_commands["WHOIS"] = &Parser::whois;
	this->_commands["QUIT"] = &Parser::quit;
	this->_commands["KICK"] = &Parser::kick;
}

Parser::~Parser() {
}

void Parser::parse(Client* client, const std::string& message) {
    std::istringstream stream(message);
    std::string command;
    std::vector<std::string> args;
    std::string token;
    std::string msg_to_client;

    try
    {
    	// Lire la commande
        if (!(stream >> command)) {
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n"; // ERR_UNKNOWNCOMMAND
	    this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	    return ;
        }

    	// Lire les arguments
        while (stream >> token) {
            args.push_back(token);
        }

        //Execution de la commande
    	std::map<std::string, commands_ptr>::iterator it = _commands.find(command);
    	if (it != _commands.end()) {
    	    (this->*(it->second))(client, args);
    	}
    	else {
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n"; // ERR_UNKNOWNCOMMAND
	    this->send_msg_to_client(client->get_client_fd(), msg_to_client);
    	}
    }
    catch(std::exception &e)
    {
	    std::cerr << "Error : " << e.what() << std::endl;
    }
}

void Parser::authenticated(Client* client) {
	std::string msg_to_client;

	if (client->get_username().empty() || client->get_realname().empty() || client->get_nickname().empty() || client->get_password() == false) {
		return ;
	}
	client->set_connected(true);
	try
	{
		msg_to_client = "001 " + client->get_nickname() + " :Welcome " + client->get_nickname() + " to my IRC server!\r\n"; // RPL_WELCOME
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
	catch(std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
	//Vérifie si le client était déjà connecté, rejoindre les channels déjà rejoind
	std::vector<Channel*> all_channels = this->_serv->get_all_channel();
	for (std::vector<Channel*>::const_iterator it = all_channels.begin(); it != all_channels.end(); it++) {
		std::vector<std::vector<Client*> > all_clients = (*it)->get_client_in_channel();
		for (std::vector<Client*>::const_iterator it_cl = all_clients[0].begin(); it_cl != all_clients[0].end(); it_cl++) {
			if ((*it_cl)->get_nickname() == client->get_nickname()) {
        			msg_to_client = ":" + client->get_nickname() + " JOIN :" + (*it)->get_channel_name() + "\r\n";
        			Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				return ;
			}
		}
		for (std::vector<Client*>::const_iterator it_cl = all_clients[1].begin(); it_cl != all_clients[1].end(); it_cl++) {
			if ((*it_cl)->get_nickname() == client->get_nickname()) {
        			msg_to_client = ":" + client->get_nickname() + " JOIN :" + (*it)->get_channel_name() + "\r\n";
        			Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				return ;
			}
		}
	}
}

void Parser::send_msg_to_client(const int client_fd, const std::string& message){
	if(send(client_fd, message.c_str(), message.size(), 0) < 0)
		throw std::runtime_error("Error while sending message to client");
}
