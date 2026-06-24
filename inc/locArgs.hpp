#ifndef LOCARGS_HPP
#define LOCARGS_HPP

#include "include.hpp"
#include "../inc/serverConfig.hpp"

void	setArgPath(Location &location, std::string &arg, size_t &i);
void	setArgRoot(Location &location, std::string &arg, size_t &i);
void	setArgMethods(Location &location, std::string &arg, size_t &i);
void	setArgIndex(Location &location, std::string &arg, size_t &i);
void	setArgAutoIndex(Location &location, std::string &arg, size_t &i);
void	setArgRet(Location &location, std::string &arg, size_t &i);
void	setArgCgiPath(Location &location, std::string &arg, size_t &i);
void	setArgCgiExt(Location &location, std::string &arg, size_t &i);
void	setArgUploadStore(Location &location, std::string &arg, size_t &i);

#endif
