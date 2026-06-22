/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haoky <haoky@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 15:38:21 by haoky             #+#    #+#             */
/*   Updated: 2026/06/18 15:44:00 by haoky            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/serverConfig.hpp"

ServerConfig::ServerConfig()
{
}

ServerConfig::ServerConfig(const ServerConfig &src)
{
	*this = src;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &src)
{
	if (this != &src)
	{
		_ports = src._ports;
		_host = src._host;
		_server_names = src._server_names;
		_client_max_body_size = src._client_max_body_size;
		_error_pages = src._error_pages;
		_locations = src._locations;
	}

	return *this;
}

ServerConfig::~ServerConfig()
{
}
