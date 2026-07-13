#ifndef SERVERARGS_HPP
#define SERVERARGS_HPP

#include "serverConfig.hpp"


void	setServerListen(ServerConfig &server, std::vector<std::string> &args);
void	setServerName(ServerConfig &server, const std::vector<std::string> &args);
void	setServerRoot(ServerConfig &server, const std::vector<std::string> &args);
void	setServerIndex(ServerConfig &server, const std::vector<std::string> &args);
void	setServerErrorPage(ServerConfig &server, const std::vector<std::string> &args);
void	setServerBodySize(ServerConfig &server, const std::vector<std::string> &args);

#endif