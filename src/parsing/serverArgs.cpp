#include "../../inc/serverArgs..hpp"

void	setServerListen(ServerConfig &server, const std::vector<std::string> &args)
{
	Listen	listen;
	size_t	pos;

	if (args.size() != 2)
		throw std::runtime_error("Error: listen must have one argument");
	if (checkDuplicateListen(server._listen))
		throw std::runtime_error("Error: duplicate listen in server block");
	
	pos = args[1].find(':');
	if (pos == std::string::npos)
	{
		listen._host = "0.0.0.0";
		listen._port = args[1];
	}
	else
	{
		listen._host = args[1].substr(0, pos);
		listen._port = args[1].substr(pos + 1);
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
	for (size_t i = 1, i < args.size(); i++)
	{
		if (args[i].epmty())
			throw std::runtime_error("Error: index can't have empty argument");
		server.setIndex(args[i]);
	}
}

void	setServerErrorPage(ServerConfig &server, const std::vector<std::string> &args)
{
	
}

void	setServerBodySize(ServerConfig &server, const std::vector<std::string> &args)
{

}