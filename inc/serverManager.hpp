#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "include.hpp"
# include "serverConfig.hpp"
# include "client.hpp"
# include "utilsError.hpp"

/*
Permet d'orchestrer la boucle principale du serveur (poll) pour gérer les cas de
connexions simultanées, en surveillant toutes les sockets pour savoir instantanément
quand un nouveau client "toque à la porte" ou quand un client déjà connecté envoie
des données, le tout sans jamais bloquer le programme.
*/

class ServerManager
{
	private:
		std::vector<ServerConfig>	_configs;
		std::vector<struct pollfd>	_pollfds;
		std::map<int, Client*>		_clients; // Liste des clients actifs

	public:
		ServerManager(std::vector<ServerConfig> configs);
		~ServerManager();

		void	initServers();	// Création des sockets listen
		void	run();			// La boucle poll()
		
	private:
		void	_acceptNewConnection(int server_fd);
		void	_handleClientData(int client_fd);
		void	_sendResponse(int client_fd);
		void	_removeClient(size_t idx);
};

#endif