#include "Server.hpp"

Server::Server(std::string port, std::string passwd) : _port(port), _passwd(passwd) {
    this->_socket_fd = create_socket();
    fcntl(this->_socket_fd, F_SETFL, O_NONBLOCK);
    pollfd serv = {this->_socket_fd, POLLIN, 0};
    this->_poll_fd.push_back(serv);
    _running = true;
}

Server::~Server() {
    close(this->_socket_fd);
    for(std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++) {
        close(it->second->get_client_fd());
        delete it->second;
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
        if (poll(this->_poll_fd.begin().base(), this->_poll_fd.size(), -1) < 0)
            throw std::runtime_error("error while polling");
        //iterer sur chaque fd disponible et vérifier les évenements disponible
        for (std::vector<pollfd>::iterator it = _poll_fd.begin(); it != _poll_fd.end(); it++) {
            //Aucun évenement
            if ((*it).revents == 0)
                continue ;
            //Evenement connection/message
            if (((*it).revents & POLLIN) == POLLIN)
            {
                if ((*it).fd == _socket_fd)
                {
                    this->client_connect();
                    break ;
                }
                this->client_message((*it).fd);
            }
            //Evenement deconnection
            if (((*it).revents & POLLHUP) == POLLHUP)
            {
                this->client_disconnect((*it).fd);
                break;
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
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    pollfd client = {this->_socket_fd, POLLIN, 0};
    this->_poll_fd.push_back(client);

    //Crée et stocker le client
    Client* new_client = new Client(client_fd);
    _clients.insert(std::make_pair(client_fd, new_client));

    std::cout << "client connected with fd = " << client_fd << std::endl;
}

void Server::client_disconnect(int client_fd) {
    for(std::vector<pollfd>::iterator it = this->_poll_fd.begin(); it != this->_poll_fd.end(); it++) {
        if (it->fd == client_fd)
        {
            this->_poll_fd.erase(it);
            break ;
        }
    }
    _clients.erase(client_fd);
    delete _client[client_fd];
    close(client_fd);
}

void Server::client_message(int client_fd) {
    client_fd++;
}
