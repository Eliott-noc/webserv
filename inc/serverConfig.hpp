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
		std::string					_index;
		bool						_auto_index;
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

		const Location				*getLocationForPath(std::string const &path);
		
		std::vector<int>			getPort() const;
		std::string					getHost() const;
		std::string					getRoot() const;
		bool						getAutoIndex() const;
		std::string					getIndex() const;
		std::vector<std::string>	getServerNames() const;
		size_t						getClientMaxBodySize() const;
		std::map<int, std::string>	getErrorPages() const;
		std::vector<Location>		getLocations() const;

		void						setPorts(const std::vector<int> &ports);
		void						setHost(const std::string &host);
		void						setRoot(const std::string &root);
		void						setIndex(const std::string &index);
		void						setAutoIndex(const bool &auto_index);
		void						setServerNames(const std::vector<std::string> &server_names);
		void						setClientMaxBodySize(const size_t &error_pages);
		void						setErrorPages(const std::map<int, std::string> &error_pages);
		void						setLocations(const std::vector<Location> &locations);

};

#endif
