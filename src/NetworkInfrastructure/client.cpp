#include "client.hpp"

Client::Client(int client_fd) : fd(client_fd), request(client_fd){

}

Client::Client(const Client &other) : request(other.fd){
	*this = other;
}

Client::~Client(){
	
}

void Client::reset() {
	this->request = Request(this->fd);
	this->response = Response();
}

Client& Client::operator=(const Client &src){
	if (this != &src){
		this->config = src.config;
		this->fd = src.fd;
		this->raw_request_buffer = src.raw_request_buffer;
		this->request = src.request;
		this->response = src.response;
	}
	return *this;
}
