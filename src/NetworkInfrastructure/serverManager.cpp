#include "serverManager.hpp"
#define R "\033[31m"
#define G "\033[32m"
#define Y "\033[33m"
#define C "\033[36m"
#define RESET "\033[0m"
#define TIMEOUT_CLIENT 3.0

ServerManager::ServerManager(std::vector<ServerConfig> configs) : _configs(configs){
	std::cout << G << "[INFO] ServerManager initialized" << RESET << std::endl;
}

ServerManager::~ServerManager(){
	for (size_t i = 0; i < _pollfds.size(); i++){
		close(_pollfds[i].fd);
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it){
		delete it->second;
	}
}

void ServerManager::initServers(){
	std::cout << G << "[INFO] Initializing server sockets..." << RESET << std::endl;
	for (size_t i = 0; i < _configs.size(); i++){
		int err_code;
		const std::vector<Listen>& listens = _configs[i].getListens();
		for (size_t j = 0; j < listens.size(); j++){
			int port = listens[j]._port;
			std::string host = listens[j]._host;
			std::cout << Y <<"[INFO] Preparing interface: " 
					<< (host.empty() ? "0.0.0.0" : host) << ":" << port << "..." << RESET << std::endl;
			int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
			if (listen_fd < 0){
				err_code = errno;
				std::cerr << R << "[ERROR] Socket creation failed on port " << port << RESET << std::endl;
				printPortErr(err_code, port);
				continue;
			}
			int opt = 1;
			if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
				err_code = errno;
				std::cerr << R << "[ERROR] setsockopt failed on socket fd: " << listen_fd << RESET << std::endl;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0){
				err_code = errno;
				std::cerr << R << "[ERROR] fcntl failed on socket fd: " << listen_fd << RESET << std::endl;
				printPortErr(err_code, port);
				close(listen_fd);
				continue;
			}
			std::stringstream ss;
			ss << port;
			std::string port_str = ss.str();

			struct addrinfo hints;
			struct addrinfo* res = NULL;

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;

			const char* node = NULL;
			if (!host.empty() && host != "0.0.0.0" && host != "*"){
				node = host.c_str();
			}
			int status = getaddrinfo(node, port_str.c_str(), &hints, &res);
			if (status != 0){
				std::cerr << R << "[ERROR] getaddrinfo resolution failed - Error details: " << gai_strerror(status) << RESET << std::endl;
				close(listen_fd);
				continue;
			}
			if (bind(listen_fd, res->ai_addr, res->ai_addrlen) < 0){
				err_code = errno;
				std::cerr << R << "[ERROR] Bind failed on socket fd " << listen_fd << " for port " << port << RESET << std::endl;
				printPortErr(err_code, port);
				freeaddrinfo(res);
				close(listen_fd);
				continue;
			}
			freeaddrinfo(res);

			if (listen(listen_fd, 5) < 0){
				err_code = errno;
				std::cerr << R << "[ERROR] listen() command failed on socket fd: " << listen_fd << RESET << std::endl;
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

			std::cout << G << "[INFO] Successfully initialized server bound to FD: " << listen_fd 
						<< " (port: " << port << ")" << RESET << std::endl;
		}
	}
}

void ServerManager::run() {
	initServers();
	size_t _listeningCount = _pollfds.size();
	if (_listeningCount == 0) {
		std::cerr << R << "[FATAL] No server sockets could be created. Shutting down." << RESET << std::endl;
		return;
	}
	std::cout << G << "[INFO] Main loop started. Listening on " << _listeningCount << " socket(s)..." << RESET << std::endl;

	while (true) {
		size_t nfds = _pollfds.size();
		int ready = poll(&_pollfds[0], nfds, 1000);

		if (ready < 0) {
			if (errno == EINTR) continue;
			std::cerr << R << "[FATAL] poll() error!" << RESET << std::endl;
			return;
		}

		int checked = 0;
		for (size_t i = 0; i < nfds; i++) {
			if (checked >= ready && i >= _listeningCount) break;

			if (_pollfds[i].revents & POLLIN) {
				checked++;
				if (i < _listeningCount) {
					_acceptNewConnection(_pollfds[i].fd);
					nfds = _pollfds.size();
					continue;
				} else {
					char buffer[8192];
					ssize_t count = recv(_pollfds[i].fd, buffer, sizeof(buffer), 0);

					if (count <= 0) {
						_removeClient(i);
						nfds--; i--;
						continue;
					}

					Client* client = _clients[_pollfds[i].fd];
					gettimeofday(&(client->last_activ), NULL);

					client->raw_request_buffer.append(buffer, count);

					int parse_status = client->request.parse(client->raw_request_buffer, client->config->getClientMaxBodySize());

					if (parse_status == 1) {
						continue;
					} else {
						if (parse_status == 200) {
							client->response.makeResponse(client->request, *(client->config));
						} else {
							client->response.buildErrorPage(parse_status, *(client->config), NULL);
						}
						_pollfds[i].events = POLLOUT;
					}
				}
			} 
			else if (_pollfds[i].revents & POLLOUT) {
				checked++;
				Client* client = _clients[_pollfds[i].fd];

				client->response.sendResponse(_pollfds[i].fd);

				if (client->response.isFinished()) {
					gettimeofday(&(client->last_activ), NULL);

					if (client->request.getKeepAlive() == false) {
						_removeClient(i);
						nfds--; i--;
					} else {
						client->reset(); 
						_pollfds[i].events = POLLIN;

						if (!client->raw_request_buffer.empty()) {
							int next_status = client->request.parse(client->raw_request_buffer, client->config->getClientMaxBodySize());
							if (next_status != 1) {
								if (next_status == 200)
									client->response.makeResponse(client->request, *(client->config));
								else
									client->response.buildErrorPage(next_status, *(client->config), NULL);
								_pollfds[i].events = POLLOUT;
							}
						}
					}
				}
			} 
			else if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				checked++;
				_removeClient(i);
				nfds--; i--;
			}
		}

		for (size_t i = _listeningCount; i < _pollfds.size(); i++) {
			if (_checkTimeouts(*_clients[_pollfds[i].fd]) == 1) {
				std::cout << Y << "[DEBUG] Timeout reached for FD: " << _pollfds[i].fd << RESET << std::endl;
				_removeClient(i);
				nfds = _pollfds.size();
				i--;
			}
		}
	}
}

void ServerManager::_acceptNewConnection(int server_fd){
	struct sockaddr_storage	client_addr;
	socklen_t				addr_len = sizeof(client_addr);
	int						err_code;
	int						client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
	
	if (client_fd < 0){
		err_code = errno;
		std::cerr << R << "[ERROR] accept() failed on server socket fd: " << server_fd << RESET << std::endl;
		printPortErr(err_code, -2);
		return;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0){
		err_code = errno;
		std::cerr << R << "[ERROR] fcntl O_NONBLOCK failed on newly accepted client fd: " << client_fd << RESET << std::endl;
		printPortErr(err_code, -2);
		close(client_fd);
		return;
	}
	pollfd new_poll;
	new_poll.fd = client_fd;
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	_pollfds.push_back(new_poll);
	Client* _newClient = new Client(client_fd);
	gettimeofday(&_newClient->last_activ, NULL);
	_newClient->config = _listenSockets[server_fd];
	_clients[client_fd] = _newClient;

	std::cout << G << "[INFO] Accepted connection. Assigned Client FD: " << client_fd 
				<< " (associated with Listening FD: " << server_fd << ")" << RESET << std::endl;
}

void ServerManager::_removeClient(size_t idx){
	if (idx >= _pollfds.size()){
		std::cerr << Y << "[WARN] Attempted out-of-bounds client removal at index: " << idx << RESET << std::endl;
		return;
	}

	int fd_to_remove = _pollfds[idx].fd;
	std::cout << G << "[INFO] Closing and removing client connection associated with FD: " << fd_to_remove << RESET << std::endl;
	
	close(fd_to_remove);
	std::map<int, Client*>::iterator it = _clients.find(fd_to_remove);
	if (it != _clients.end()){
		std::cout << Y << "[DEBUG] Deleting client memory allocation associated with FD: " << fd_to_remove << RESET << std::endl;
		delete it->second;
		_clients.erase(it);
	}
	
	if (idx < _pollfds.size() - 1){
		std::cout << Y << "[DEBUG] Swapping deleted index " << idx << " (fd: " << fd_to_remove 
				<< ") with last element at index " << (_pollfds.size() - 1) 
				<< " (fd: " << _pollfds.back().fd << ")" << RESET << std::endl;
		_pollfds[idx] = _pollfds.back();
	}
	_pollfds.pop_back();
	std::cout << Y << "[DEBUG] Client removal completed. Active monitored socket count is now: " << _pollfds.size() << RESET << std::endl;
}

int	ServerManager::_checkTimeouts(Client& client){
	struct timeval now;
	gettimeofday(&now, NULL);
	double elapsed_secs = (now.tv_sec - client.last_activ.tv_sec) + (now.tv_usec - client.last_activ.tv_usec) / 1000000.0;
	double timeout = TIMEOUT_CLIENT;
	if (elapsed_secs > timeout)
		return 1;
	else
		return 0;
}
