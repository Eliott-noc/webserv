#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "../inc/serverConfig.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <map>


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
