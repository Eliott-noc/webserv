#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "../inc/serverConfig.hpp"

std::vector<ServerConfig>	parseConfig(const std::string &filename);

#endif
