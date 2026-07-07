#ifndef UTILS_HPP
#define UTILS_HPP

#include "locArgs.hpp"

int	checkDuplicateIndex(const std::vector<std::string> &args);
int	checkDuplicateMethods(const std::string &arg, t_methods *methods);
int	isServKeyword(const std::string &str);
int	isLocKeyword(const std::string &str);

#endif