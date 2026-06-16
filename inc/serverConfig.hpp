#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "webserv.hpp"
#include "location.hpp"

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
		ServerConfig(const ServerConfig &src);
		~ServerConfig();
		
		ServerConfig	&operator=(const ServerConfig &src);
		// Getters et Setters
		// ...
		void addLocation(const Location &loc);
};

#endif