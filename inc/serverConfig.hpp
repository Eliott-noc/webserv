#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "include.hpp"
# include "location.hpp"

/*
permet de regrouper l'identité complète d'un serveur virtuel pour gérer les cas où
ton programme doit écouter sur plusieurs ports en même temps (ex: 8080 et 4242) ou
répondre à des noms de domaine différents (server_name), tout en gardant en mémoire
les pages d'erreur personnalisées à afficher en cas de problème.
*/

class ServerConfig
{
	private:
		int							_port;
		std::string					_host;
		std::vector<std::string>	_server_names;
		size_t						_client_max_body_size;
		std::map<int, std::string>	_error_pages;
		std::vector<Location>		_locations;
	
	public:
		ServerConfig();
		ServerConfig(const ServerConfig &other);
		~ServerConfig();
		
		ServerConfig	&operator=(const ServerConfig &other);
		// Getters et Setters
		// ...
		void addLocation(const Location &loc);
};

#endif