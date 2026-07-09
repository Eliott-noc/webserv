#include "serverManager.hpp"

ServerManager::ServerManager(std::vector<ServerConfig> configs){

}

ServerManager::~ServerManager(){

}

void ServerManager::initServers(){
	for (size_t i = 0; i < _configs.size(); i++){
		int err_code, port;
		for (size_t j = 0; j < _configs[i].getPort().size(); j++){
			port = _configs[i].getPort(j);
			int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_fd < 0){
				err_code = errno;
				printPortErr(err_code, port);
				continue;
			}
			int opt = 1;
			if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
				err_code = errno;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0){
				err_code = errno;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			struct sockaddr_in serv_addr;
			bzero((char*)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = INADDR_ANY;
			serv_addr.sin_port = htons(port);
			if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
				err_code = errno;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			if (listen(listen_fd, 5) < 0){
				err_code = errno;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			pollfd new_poll;
			new_poll.fd = listen_fd;
			new_poll.events = POLLIN;
			new_poll.revents = 0;
			_pollfds.push_back(new_poll);
			_listenSockets[listen_fd] = &_configs[i];
		}
	}
}

void ServerManager::run(){
	initServers();
	size_t	_listeningCount = _pollfds.size();
	if (_listeningCount == 0){
		std::cerr << "Fatal: No socket was created" << std::endl;
		// exit // return;
	}
	while(true){
		size_t	nfds = _pollfds.size();
		int		err_code;
		int		ready = poll(&_pollfds[0], nfds, 1000);
		if (ready < 0){
			err_code = errno;
			printPortErr(err_code, -2);
			//exit //return
		}
		else if (ready == 0)
			continue;
		else{
			int checked = 0;
			for (size_t i = 0; i < nfds; i++){
				if (checked == ready)
					break;
				if (_pollfds[i].revents & POLLIN){
					checked++;
					if (i < _listeningCount){
						_acceptNewConnection(_pollfds[i].fd);
						continue;
					}
					else{
						char buffer[8192];
						ssize_t count = recv(_pollfds[i].fd, buffer, sizeof(buffer), 0);
						if (count == -1){
							err_code = errno;
							if (err_code != EAGAIN && err_code != EWOULDBLOCK){
								printPortErr(err_code, -2);
								_removeClient(i);
								nfds--;
								i--;
							}
						}
						else if (count == 0){
							_removeClient(i);
							nfds--;
							i--;
						}
						else {
							int client_fd = _pollfds[i].fd;
							Client* client = _clients[client_fd];
							size_t body_limit = client->config->getClientMaxBodySize();
							std::string chunk(buffer, count);
							int parse_status = client->request.parse(chunk, body_limit);
							if (parse_status == 1)
								client->request_is_complete = false;
							else {
								client->request_is_complete = true;
								if (parse_status == 200)
									client->response.makeResponse(client->request, *(client->config));
								else
									client->response.buildErrorPage(parse_status, *(client->config));
								_pollfds[i].events = POLLOUT;
							}
						}
					}
				}
				else if (_pollfds[i].revents & POLLOUT){
					checked++;
					int client_fd = _pollfds[i].fd;
					Client* client = _clients[client_fd];

					client->response.sendResponse(client_fd);
					if (client->response.isFinished()){
						_removeClient(client_fd);
						nfds--;
						i--;
					}
				}
				else if (_pollfds[i].revents & POLLERR || _pollfds[i].revents & POLLHUP || _pollfds[i].revents & POLLNVAL){
					checked++;
					_removeClient(i);
					nfds--;
					i--;
				}
			}
		}
	}
}

void ServerManager::_acceptNewConnection(int server_fd){
	struct sockaddr_in	client_addr;
	socklen_t			addr_len = sizeof(client_addr);
	int					err_code;
	int					client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
	
	if (client_fd < 0){
		err_code = errno;
		printPortErr(err_code, -2);
		return;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0){
		err_code = errno;
		printPortErr(err_code, -2);
		close(client_fd);
		return;
	}
	pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	_pollfds.push_back(new_poll);
	Client* _newClient = new Client(server_fd);
	_newClient->config = _listenSockets[server_fd];
	_clients[server_fd] = _newClient;
}

void ServerManager::_handleClientData(int server_fd){
	
}

void ServerManager::_sendResponse(int server_fd){
	
}

void ServerManager::_removeClient(size_t idx){
	close(_pollfds[idx].fd);
	_pollfds[idx] = _pollfds[_pollfds.size() - 1];
	_pollfds.pop_back();
	_clients.erase(_pollfds[_pollfds.size() - 1].fd);
}