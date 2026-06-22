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
		std::vector<int>			_ports;
		std::string					_host;
		std::string					_root;
		int							_AutoIndex;
		std::string					_Index;
		std::vector<std::string>	_server_names;
		size_t						_client_max_body_size;
		std::map<int, std::string>	_error_pages;
		std::vector<Location>		_locations;

	public:
		ServerConfig();
		ServerConfig(const ServerConfig &other);
		~ServerConfig();
		
		ServerConfig				&operator=(const ServerConfig &other);

		void						addLocation(const Location &loc);

		const Location				*getLocationForPath(std::string path);
		
		std::vector<int>			getPort() const;
		std::string					getHost() const;
		std::string					getRoot() const;
		bool						getAutoIndex() const;
		std::string					getIndex() const;
		std::vector<std::string>	getServerNames() const;
		size_t						getClientMaxBodySize() const;
		std::map<int, std::string>	getErrorPages() const;
		std::vector<Location>		getLocations() const;
};

#endif
