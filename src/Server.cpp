#include "../include/Server.hpp"
#include "../include/Parser.hpp"

void Server::signal_handler(int signum) {
    (void)signum;
    throw(std::runtime_error("Signal received"));
}

Server::Server(std::string port, std::string passwd) : _port(port), _passwd(passwd) {
    this->_socket_fd = create_socket();
    fcntl(this->_socket_fd, F_SETFL, O_NONBLOCK);
    pollfd serv = {this->_socket_fd, POLLIN, 0};
    this->_poll_fd.push_back(serv);
    this->_running = true;
    this->_parser = new Parser(this);
}

Server::~Server() {
    close(this->_socket_fd);
    for(std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++) {
        close(it->second->get_client_fd());
        delete it->second;
    }
    delete _parser;
    for(size_t i = 0; i != this->_channels.size(); i++) {
        delete this->_channels[i];
    }
};

int Server::create_socket() {
    struct sockaddr_in6 addr;
    int socket_fd;
    int optval = 0;

    //création du socket
    socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0)
        throw std::runtime_error("error while creating socket");

    //désactiver IPV6_V6ONLY pour accepter les connexions ipv4 mappé
    if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0) {
        close(socket_fd);
        throw std::runtime_error("error while disable IPV6_V6ONLY error");
    }

    //permettre la reutilisation du socket
    optval = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        close(socket_fd);
        throw std::runtime_error("error while setting reusable");
    }

    //rassemblement des info necessaire pour bind
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(atoi(this->_port.c_str()));

    //Lier socket à l'addresse ip + port
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(socket_fd);
        throw std::runtime_error("error while binding sockfd");
    }

    //Mettre en écoute le socket
    if (listen(socket_fd, MAX_CLIENTS) < 0) {
        close(socket_fd);
        throw std::runtime_error("error while listening");
    }

    return socket_fd;
}

void Server::start() {
    while(_running) {
        //verifier si il y a une erreur de poll
        int tmp = poll(this->_poll_fd.begin().base(), this->_poll_fd.size(), -1);
	if(tmp < 0)
            throw std::runtime_error("error while polling");
        //iterer sur chaque fd disponible et vérifier les évenements disponible
        for (std::vector<pollfd>::iterator it = _poll_fd.begin(); it != _poll_fd.end(); it++) {
            //Aucun évenement
            if ((*it).revents == 0) {
                continue ;
	    }
            //Evenement connection/message
            if (((*it).revents & POLLIN) == POLLIN)
            {
                if ((*it).fd == _socket_fd)
                {
                    this->client_connect();
                    break ;
                }
                this->client_message(it);
            }
	    if (it == _poll_fd.end())
		    break;
	    //La déconnection est effectué dans read_message. Ici, c'est juste pour arreter la boucle quand il se déconnecte afin de pas faire crash le serv
	    if (((*it).revents & POLLHUP) == POLLHUP) {
		this->client_disconnect((*it).fd);
                break ;
	    }
        }
    }
}

void Server::client_connect() {
    int client_fd;
    struct sockaddr_in6 client_addr = {};
    socklen_t addr_len = sizeof(client_addr);

    //accepte la connection du client
    client_fd = accept(this->_socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
        throw std::runtime_error("error while accept client connection");
    //Descripteur des client non blocant + Ajouter le client a poll
    pollfd client = {client_fd, POLLIN | POLLHUP, 0};
    this->_poll_fd.push_back(client);

    //Crée et stocker le client
    Client* new_client = new Client(client_fd);
    _clients.insert(std::make_pair(client_fd, new_client));

    std::cout << "client connected with fd = " << client_fd << std::endl;
}

void Server::client_disconnect(int client_fd) {
    std::cout << "Client disconnected with fd = " << client_fd << std::endl;
    for(std::vector<pollfd>::iterator it = this->_poll_fd.begin(); it != this->_poll_fd.end();) {
        if (it->fd == client_fd)
        {
            it = this->_poll_fd.erase(it);
            break ;
        }
        else {
            it++;
        }
    }
    delete _clients[client_fd];
    _clients.erase(client_fd);
    close(client_fd);
}

void Server::client_message(std::vector<pollfd>::iterator it_client) {
	std::string message;
	char buffer[1024];
	ssize_t bytes;

	memset(buffer, 0, sizeof(buffer));
	while(!strstr(buffer, "\n"))
	{
		memset(buffer, 0, sizeof(buffer));
		bytes = recv((*it_client).fd, buffer, 1023, 0);
		if (bytes == 0) {
			this->client_disconnect((*it_client).fd);
			it_client = _poll_fd.end();
			return ;
		}
		else if (bytes < 0) {
			std::cerr << "Error while reading CLIENT socket" << std::endl;
			return ;
		}
		message.append(buffer);
	}
	std::cout << "client message : " << message << std::endl;
	//Utiliser la class parser pour gerer le message
    	if (message == "\n")
        	return;
	this->_parser->parse(this->_clients.at((*it_client).fd), message);
}

Channel* Server::create_channel(Client* client, std::vector<std::string> channel, std::vector<std::string> key, size_t i) {
	Channel* new_channel = NULL;
	if(key.empty()) {
		new_channel = new Channel(channel[i], "", client, this);
		this->set_channels(new_channel);
		client->set_channels(new_channel);
	}
	else {
		new_channel = new Channel(channel[i], key[i], client, this);
		this->set_channels(new_channel);
		client->set_channels(new_channel);
	}
	return new_channel;
}
//getter
Client* Server::get_client_nickname(const std::string& nickname) const {
	for (std::map<int, Client*>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); it++){
		if (nickname.compare(it->second->get_nickname()) == 0)
			return it->second;
	}
	return NULL;
}

std::string Server::get_password() const {
	return(this->_passwd);
}

Channel* Server::get_channel(const std::string& channel) const {
	for(std::vector<Channel*>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); it++) {
		if(channel.compare((*it)->get_channel_name()) == 0)
			return *it;
	}
	return NULL;
}

std::vector<Channel*> Server::get_all_channel() const {
	return(this->_channels);
}

//setter
void Server::set_channels(Channel* channel) {
	this->_channels.push_back(channel);
}
