#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "CGIHandler.hpp"
# include "client.hpp"
# include "configParser.hpp"
# include "location.hpp"
# include "request.hpp"
# include "response.hpp"
# include "serverConfig.hpp"
# include "serverManager.hpp"

// Parsing : ConfigParser, ServerConfig et Location.
// NetworkInfrastructure : ServerManager et Client.
// Translator : Request, Response et CGIHandler.


class Webserv
{
	private:
		std::vector<ServerConfig> _servers;

	public:
		Webserv();
		Webserv(std::vector<ServerConfig> servers);
		~Webserv();
};

#endif
