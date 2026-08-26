#include "../../inc/include.hpp"
#include "../../inc/locArgs.hpp"
#include "../../inc/serverConfig.hpp"

bool	checkInt(const std::string &str)
{
	if (str.empty())
		return false;

	size_t i = 0;

	if (str[0] == '+' || str[0] == '-')
	{
		if (str.size() == 1)
			return false;
		i = 1;
	}
	while (i < str.size())
	{
		if (!std::isdigit(str[i]))
			return false;
		i++;
	}
	return true;
}

bool	checkLocation(Location *location, const ServerConfig &server)
{
	if (location->getIndex().empty())
		location->setIndex(server.getIndex());

	if (location->getRoot().empty())
		location->setRoot(server.getRoot());

	if (location->getClientMaxBodySize() == 0)
		location->setClientMaxBodySize(server.getClientMaxBodySize());

	if (location->getErrorPages().empty())
		location->setErrorPages(server.getErrorPages());
	return 0;
}

bool	isValidHost(const std::string &host)
{
	if (host.empty())
		return false;

	int parts = 0;
	size_t start = 0;

	while (start < host.size())
	{
		size_t end = host.find('.', start);
		std::string part;

		if (end == std::string::npos)
			part = host.substr(start);
		else
			part = host.substr(start, end - start);

		if (part.empty())
			return false;

		for (size_t i = 0; i < part.size(); i++)
		{
			if (!std::isdigit(static_cast<unsigned char>(part[i])))
				return false;
		}

		if (part.size() > 1 && part[0] == '0')
			return false;

		int value = std::atoi(part.c_str());

		if (value < 0 || value > 255)
			return false;

		parts++;

		if (end == std::string::npos)
			break ;

		start = end + 1;
	}

	return (parts == 4);
}

bool	isValidPort(const std::string &str)
{
	if (str.empty())
		return false;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return false;
	}
	int port = std::atoi(str.c_str());

	return (port >= 1 && port <= 65535);
}

size_t parseSize(const std::string& str)
{
	size_t multiplier = 1;
	std::string number = str;

	char unit = str[str.size() - 1];

	if (unit == 'K' || unit == 'k')
	{
		multiplier = 1024;
		number = str.substr(0, str.size() - 1);
	}
	else if (unit == 'M' || unit == 'm')
	{
		multiplier = 1024 * 1024;
		number = str.substr(0, str.size() - 1);
	}
	else if (unit == 'G' || unit == 'g')
	{
		multiplier = 1024 * 1024 * 1024;
		number = str.substr(0, str.size() - 1);
	}

	if (!checkInt(number))
		throw std::runtime_error("Invalid body size");

	return std::atoi(number.c_str()) * multiplier;
}


int	checkDuplicateListen(const std::vector<Listen> &listen_block)
{
	for (size_t i = 0; i < listen_block.size(); i++)
	{
		if (i + 1 < listen_block.size() - 1)
		{
			for (size_t j = i + 1; j < listen_block.size(); j++)
			{
				if ((listen_block[i]._host == listen_block[j]._host) && (listen_block[i]._port == listen_block[j]._port))
					return 1;
			}
		}
	}
	return 0;
}

int	checkDuplicateIndex(const std::vector<std::string> &args)
{
	for (size_t i = 0; i < args.size(); i++)
	{
		for (size_t j = i + 1; j < args.size(); j++)
		{
			if (args[i] == args[j])
				return 1;
		}
	}
	return 0;
}

int	checkDuplicateMethods(const std::string &arg, t_methods *methods)
{
	if (arg == "GET")
	{
		if (methods->_get == 1)
			return 1;
		methods->_get = 1;
	}
	else if (arg == "POST")
	{
		if (methods->_post == 1)
			return 1;
		methods->_post = 1;
	}
	else if (arg == "DELETE")
	{
		if (methods->_delete == 1)
			return 1;
		methods->_delete = 1;
	}
	return 0;
}

int	isServKeyword(const std::string &str)
{
	if (str == "listen" || str == "server_name" || str == "root"
		|| str == "index" || str == "error_page" || str == "client_max_body_size"
		|| str == "host" || str == "location")
		return 1;

	return 0;
}

int	isLocKeyword(const std::string &str)
{
	if (str == "autoindex" || str == "allow_methods" || str == "root"
		|| str == "index" || str == "return" || str == "upload_dir"
		|| str == "upload_store" || str == "alias" || str == "cgi"
		|| str == "cgi_ext" || str == "cgi_path" || str == "error_page"
		|| str == "client_max_body_size")
		return 1;

	return 0;
}

bool	isServerKeyword(const std::string& token)
{
	if (token == "listen" || token == "host" || token == "server_name" || 
		token == "root" || token == "index" || token == "error_page" || 
		token == "client_max_body_size" || token == "return" || token == "location")
		return true;
	return false;
}
