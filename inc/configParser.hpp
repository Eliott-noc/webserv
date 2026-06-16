#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.hpp"
# include "serverConfig.hpp"

class ConfigParser
{
	public:
		// Prend le chemin du fichier et renvoie la liste des serveurs configurés
		static std::vector<ServerConfig> parse(std::string filename);
};

#endif