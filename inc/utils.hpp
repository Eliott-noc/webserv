#ifndef UTILS_HPP
#define UTILS_HPP

#include "locArgs.hpp"

bool	checkInt(const std::string &str);
bool	isValidPort(const std::string &str);
bool	isValidHost(const std::string &host);
size_t	parseSize(const std::string& str);
int		checkDuplicateListen(const std::vector<Listen> &listen_block);
int		checkDuplicateIndex(const std::vector<std::string> &args);
int		checkDuplicateMethods(const std::string &arg, t_methods *methods);
int		isServKeyword(const std::string &str);
int		isLocKeyword(const std::string &str);
bool	checkLocation(Location *Location, const ServerConfig &server);

#endif
