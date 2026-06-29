#include "client.hpp"

Client::Client(int client_fd) : request(client_fd), fd(client_fd), request_is_complete(0), response_is_ready(0){

}

Client::Client(const Client &other) : request(other.fd){

}

Client::~Client(){
	
}