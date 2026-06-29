#ifndef LOCARGS_HPP
#define LOCARGS_HPP

#include "include.hpp"
#include "../inc/serverConfig.hpp"

void	setArgPath(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgRoot(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgMethods(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgIndex(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgAutoIndex(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgRet(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgCgiPath(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgCgiExt(Location &location, const std::vector<std::string> &args, size_t &i);
void	setArgUploadStore(Location &location, const std::vector<std::string> &args, size_t &i);

#endif
