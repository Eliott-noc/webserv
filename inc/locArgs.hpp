#ifndef LOCARGS_HPP
#define LOCARGS_HPP

#include "include.hpp"
#include "../inc/serverConfig.hpp"

void	setArgPath(Location &location, const std::string &path);
void	setArgRoot(Location &location, const std::vector<std::string> &args);
void	setArgMethods(Location &location, std::vector<std::string> &args);
void	setArgIndex(Location &location, const std::vector<std::string> &args);
void	setArgAutoIndex(Location &location, const std::vector<std::string> &args);
void	setArgRet(Location &location, const std::vector<std::string> &args);
void	setArgCgiPath(Location &location, const std::vector<std::string> &args);
void	setArgCgiExt(Location &location, const std::vector<std::string> &args);
void	setArgUploadStore(Location &location, const std::vector<std::string> &args);

#endif
