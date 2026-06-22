#include "../../inc/serverConfig.hpp"

ServerConfig::ServerConfig() 
{

}

ServerConfig::ServerConfig(const ServerConfig &other)
{
	*this = other;
}

ServerConfig::~ServerConfig() {}

ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		_ports = other._ports;
		_host = other._host;
		_server_names = other._server_names;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_locations = other._locations;
	}
	return *this;
}

void	ServerConfig::addLocation(const Location &loc)
{
	_locations.push_back(loc);
}

const Location	*ServerConfig::getLocationForPath(std::string const &path)
{
	const Location	*bestMatch = NULL;
	size_t			longestMatch = 0;

	for (size_t i = 0; i < _locations.size(); ++i)
	{
		std::string locPath = _locations[i].getPath();
		if (path.find(locPath) == 0)
		{
			if (locPath.length() >= longestMatch) 
			{
				longestMatch = locPath.length();
				bestMatch = &_locations[i];
			}
		}
	}
	return bestMatch;
}

std::vector<int>	ServerConfig::getPort() const
{
	return _ports;
}

std::string	ServerConfig::getHost() const
{
	return _host;
}

std::string	ServerConfig::getRoot() const
{
	return _root;
}

bool	ServerConfig::getAutoIndex() const
{
	return _AutoIndex;
}

std::string	ServerConfig::getIndex() const
{
	return _Index;
}

std::vector<std::string>	ServerConfig::getServerNames() const
{
	return _server_names;
}

size_t	ServerConfig::getClientMaxBodySize() const
{
	return _client_max_body_size;
}

std::map<int, std::string>	ServerConfig::getErrorPages() const
{
	return _error_pages;
}

std::vector<Location>	ServerConfig::getLocations() const
{
	return _locations;
}
