#include "Parser.hpp"

Parser::Parser(Server* server) : _serv(server) {
	this->_commands["NICK"] = &Parser::nick;
	this->_commands["USER"] = &Parser::user;
	this->_commands["PASS"] = &Parser::pass;
	this->_commands["PRIVMSG"] = &Parser::privmsg;
	this->_commands["JOIN"] = &Parser::join;
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
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n";
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
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n";
	    this->send_msg_to_client(client->get_client_fd(), msg_to_client);
    	}
    }
    catch(std::exception &e)
    {
	    std::cerr << "Error : " << e.what() << std::endl;
    }
}

void Parser::nick(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty() || args[0].empty()) {
		msg_to_client = "431 " + client->get_nickname() + " :Nickname not given\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	if(this->_serv->get_client_nickname(args[0]) != NULL) {
		msg_to_client = "433 " + client->get_nickname() + " :Nickname already used\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	client->set_nickname(args[0]); 
	this->authenticated(client);
}

void Parser::user(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.size() < 4) {
		msg_to_client = "461 " + client->get_nickname() + " :USER command has not enought params\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	client->set_username(args[0]);
	client->set_realname(args[3]);
	this->authenticated(client);
}

void Parser::pass(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty() || args[0].empty()) {
		msg_to_client = "461 " + client->get_nickname() + " :PASS command has not enought params\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	if (this->_serv->get_password() != args[0]) {
		msg_to_client = "464 " + client->get_nickname() + " :Wrong password\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	client->set_password(true);
	this->authenticated(client);
}

void Parser::authenticated(Client* client) {
	std::string msg_to_client;

	if (client->get_username().empty() || client->get_realname().empty() || client->get_nickname().empty() || client->get_password() == false) {
		return ;
	}
	client->set_connected(true);
	try
	{
		msg_to_client = "001 " + client->get_nickname() + " :Welcome " + client->get_nickname() + " to my IRC server!\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
	catch(std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}

void Parser::privmsg(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::string message;

	if(client->get_connected() == false) {
		return ;
	}
	if(args.empty() || args[0].empty() || args[1].empty())
	{
		msg_to_client = "461 " + client->get_nickname() + " :PRIVMSG command has not enought params\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//reconstrure le message
	for(std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); it++) {
		if (!message.empty()){
			message += " ";
		}
		message += *it;
	}
	//Envoyer message
	if (args[0][0] == '#'){
		return ;
	}
	else {
		Client* reciever = this->_serv->get_client_nickname(args[0]);
		if (reciever == NULL) {
			msg_to_client = "401 " + client->get_nickname() + " " + args[0] + " :This nickname doesn't exist\r\n";
			send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		}
		msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + args[0] + " " + message + "\r\n";
		this->send_msg_to_client(reciever->get_client_fd(), msg_to_client);
		return ;
	}

}

void Parser::join(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty() || args[0].empty()) {
		msg_to_client = "461 " + client->get_nickname() + " :JOIN command has not enought params\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
}

void Parser::send_msg_to_client(const int client_fd, const std::string& message) {
	if(send(client_fd, message.c_str(), message.size(), 0) < 0)
		throw std::runtime_error("Error while sending message to client");
}
