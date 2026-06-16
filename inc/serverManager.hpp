#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <poll.h>
# include <vector>
# include "webserv.hpp"
# include "serverConfig.hpp"
# include "client.hpp"

class ServerManager
{
	private:
		std::vector<ServerConfig>	_configs;
		std::vector<struct pollfd>	_pollfds;
		std::map<int, Client*>		_clients; // Liste des clients actifs

	public:
		ServerManager(std::vector<ServerConfig> configs);
		~ServerManager();

		void	initServers(); // Création des sockets listen
		void	run();         // La boucle poll()
		
	private:
		void	_acceptNewConnection(int server_fd);
		void	_handleClientData(int client_fd);
		void	_sendResponse(int client_fd);
};

#endif