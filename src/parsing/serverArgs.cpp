#include "../../inc/serverArgs.hpp"
#include "../../inc/serverConfig.hpp"
#include "../../inc/utils.hpp"

void	setServerListen(ServerConfig &server, std::vector<std::string> &args)
{
	Listen	listen;
	size_t	pos;

	if (args.size() != 2)
		throw std::runtime_error("Error: listen must have one argument");
	if (checkDuplicateListen(server.getListens()))
		throw std::runtime_error("Error: duplicate listen in server block");
	pos = args[1].find(':');
	if (pos == std::string::npos)
	{
		listen._host = "0.0.0.0";
		if (checkInt(args[1]))
			throw std::runtime_error("Error: Invalid port");
		listen._port = std::atoi(args[1].c_str());
	}
	else
	{
		listen._host = args[1].substr(0, pos);
		if (checkInt(args[1]))
			throw std::runtime_error("Error: Invalid port");
		listen._port = std::atoi(args[1].substr(pos + 1).c_str());
	}
	server.setListen(listen);
}

void	setServerName(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() < 2)
		throw std::runtime_error("Error: server_name must have at least one argument");
	for (size_t i = 1; i < args.size(); i++)
	{
		if (args[i].empty())
			throw std::runtime_error("Error: server_name can't have empty arguments");
		server.setServerNames(args[i]);
		//Check si 2 noms identiques = erreur ?
	}
}

void	setServerRoot(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: root must have exactly one argument");
	server.setRoot(args[1]);
}

void	setServerIndex(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() < 2)
		throw std::runtime_error("Error: index must have at least one argument");
	for (size_t i = 1; i < args.size(); i++)
	{
		if (args[i].empty())
			throw std::runtime_error("Error: index can't have empty argument");
		server.setIndex(args[i]);
	}
}

void	setServerErrorPage(ServerConfig &server, const std::vector<std::string> &args)
{
	std::string					path;
	size_t						end = args.size() - 1;
	std::map<int, std::string>	error_pages;

	if (args.size() < 2)
		throw std::runtime_error("Error: error_page must have at least one argument");
	if (!checkInt(args[end]))
		throw std::runtime_error("Error: invalid path for error_page");
	for (size_t i = 1; i < args.size(); i++)
	{
		if (!checkInt(args[i]))
			error_pages[std::atoi(args[i].c_str())] = args[end];
	}
	server.setErrorPages(error_pages);
}

void	setServerBodySize(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: client_max_body_size must have 2 arguments");
	server.setClientMaxBodySize(parseSize(args[1]));
}
