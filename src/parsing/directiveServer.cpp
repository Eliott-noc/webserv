#include "../../inc/directiveServer.hpp"

DirectiveServer::DirectiveServer()
{
	_ports = false;
	_host = false;
	_root = false;
	_index = false;
	_auto_index = false;
	_server_names = false;
	_client_max_body_size = false;
}

void	DirectiveServer::setPorts()
{
	_ports = true;
}

void	DirectiveServer::setHost()
{
	_host = true;
}

void	DirectiveServer::setRoot()
{
	_root = true;
}

void	DirectiveServer::setIndex()
{
	_index = true;
}

void	DirectiveServer::setAutoIndex()
{
	_auto_index = true;
}

void	DirectiveServer::setServerNames()
{
	_server_names = true;
}

void	DirectiveServer::setClientMaxBodySize()
{
	_client_max_body_size = true;
}