#ifndef webserv_HPP
# define webserv_HPP

#include <iostream>

typedef struct	s_serverConfig
{
	int			port;
	std::string	host;
	std::string	root;
	size_t		max_body;
}	t_serverConfig;

#endif