#include "Client.hpp"

Client::Client(int client_fd) : _client_fd(client_fd){
}

Client::~Client() {

}

int Client::get_client_fd() const {
    return (this->_client_fd);
}