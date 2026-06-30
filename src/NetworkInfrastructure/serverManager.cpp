#include "serverManager.hpp"

ServerManager::ServerManager(std::vector<ServerConfig> configs){

}

ServerManager::~ServerManager(){

}

void ServerManager::initServers(){
	for (size_t i = 0; i < _configs.size(); i++){
		int err_code, port;
		for (size_t j = 0; j < _configs[i].getPort().size(); j++){
			int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_fd < 0){
				err_code = errno;
				printPortErr(err_code, _configs[i].getPort(j));
				continue;
			}
			int opt = 1;
			if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
				std::cerr << "Fatal: setsockopt failed." << std::endl;
				close(listen_fd);
				continue;
			}
			if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0){
				err_code = errno;
				printPortErr(err_code, _configs[i].getPort(j));
				close(listen_fd);
				continue;
			}
			struct sockaddr_in serv_addr;
			bzero((char*)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = INADDR_ANY;
			serv_addr.sin_port = htons(_configs[i].getPort(j));
			if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
				err_code = errno;
				printPortErr(err_code, _configs[i].getPort(j));
				close(listen_fd);
				continue;
			}
			if (listen(listen_fd, 5) < 0){
				err_code = errno;
				printPortErr(err_code, _configs[i].getPort(j));
				close(listen_fd);
				continue;
			}
			pollfd new_poll;
			new_poll.fd = listen_fd;
			new_poll.events = POLLIN;
			new_poll.revents = 0;
			_pollfds.push_back(new_poll);
		}
	}
}

void ServerManager::run(){
	
}

void ServerManager::_acceptNewConnection(int server_fd){

}

void ServerManager::_handleClientData(int server_fd){
	
}

void ServerManager::_sendResponse(int server_fd){
	
}