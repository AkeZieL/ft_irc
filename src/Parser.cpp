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
}

void Parser::send_msg_to_client(const int client_fd, const std::string& message){
	if(send(client_fd, message.c_str(), message.size(), 0) < 0)
		throw std::runtime_error("Error while sending message to client");
}
