#include "../../inc/webserv.hpp"
#include "../../inc/serverConfig.hpp"

Webserv::Webserv()
{
}

Webserv::Webserv(std::vector<ServerConfig> servers)
{
	_servers = servers;
}

Webserv::~Webserv()
{
}
