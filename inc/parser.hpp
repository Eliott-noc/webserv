#ifndef PARSER_HPP
#define PARSER_HPP

#include "include.hpp"
#include "../inc/serverConfig.hpp"

std::vector<ServerConfig>	parseConfig(const std::string &filename);

#endif
