#include "../../inc/serverArgs.hpp"
#include "../../inc/serverConfig.hpp"
#include "../../inc/utils.hpp"

void	setServerListen(ServerConfig &server, std::vector<std::string> &args)
{
	Listen		listen;
	std::string	host;
	std::string	port;
	size_t		pos;

	if (args.size() != 2)
		throw std::runtime_error("Error: listen must have one argument");
	pos = args[1].find(':');
	if (pos == std::string::npos)
	{
		if (server.getHost().empty())
			listen._host = "0.0.0.0";
		else
			listen._host = server.getHost();
		if (!isValidPort(args[1]))
			throw std::runtime_error("Error: invalid port");
		listen._port = std::atoi(args[1].c_str());
	}
	else
	{
		host = args[1].substr(0, pos);
		port = args[1].substr(pos + 1);
		if (!isValidHost(host))
			throw std::runtime_error("Error: invalid host");
		listen._host = host;
		if (!isValidPort(port))
			throw std::runtime_error("Error: Invalid port");
		listen._port = std::atoi(port.c_str());
	}
	server.setListen(listen);
	if (checkDuplicateListen(server.getListens()))
		throw std::runtime_error("Error: duplicate listen in server block");
}

void	setServerHost(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: host must have one argument");
	if (!isValidHost(args[1]))
		throw std::runtime_error("Error: invalid host");
	server.setHost(args[1]);
}

void	setServerRet(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() != 2 && args.size() != 3)
		throw std::runtime_error("Error: return must have one or two argument");

	int	code;
	std::stringstream ss(args[1]);
	ss >> code;

	if (ss.fail() || !ss.eof())
		throw std::runtime_error("Error: invalid number");
	if (code < 200 || code > 599)
		throw std::runtime_error("Error: invalid http code");
	if (args.size() == 2)
		server.setRet(code , "");
	else
		server.setRet(code, args[2]);
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
	std::cout << "PARSING INDEX" << std::endl;
	for (size_t i = 1; i < args.size(); i++)
	{
		if (args[i].empty())
			throw std::runtime_error("Error: index can't have empty argument");
		server.setIndex(args[i]);
	}
}

void	setServerErrorPage(ServerConfig &server, const std::vector<std::string> &args)
{
	std::string	path = args.back();
	std::map<int, std::string> error_pages;

	if (path.empty() || path[0] != '/')
		throw std::runtime_error("Error: invalid path for error_page");

	for (size_t i = 1; i < args.size() - 1; i++)
	{
		if (!checkInt(args[i]))
			throw std::runtime_error("Error: invalid error code");
		int code = std::atoi(args[i].c_str());
		if (code < 400 || code > 599)
			throw std::runtime_error("Error: invalid HTTP error code");
		error_pages[code] = path;
	}
	server.setErrorPages(error_pages);
}

void	setServerBodySize(ServerConfig &server, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: client_max_body_size must have 2 arguments");
	server.setClientMaxBodySize(parseSize(args[1]));
}
