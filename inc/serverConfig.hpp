#ifndef serverConfig_HPP
# define serverConfig_HPP

#include "location.hpp"
#include <map>

class ServerConfig
{
	private:
		std::vector<int>			_ports;
		std::string					_host;
		std::vector<std::string>	_server_names;
		size_t						_client_max_body_size;
		std::map<int, std::string>	_error_pages;
		std::vector<Location>		_locations;

	public:
		ServerConfig();
		ServerConfig(const ServerConfig &src);
		ServerConfig	&operator=(const ServerConfig &src);
		~ServerConfig();

		// Getters et Setters
		// ...
		//void addLocation(const Location &loc);
};

#endif
