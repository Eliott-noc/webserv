#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "webserv.hpp"
# include "serverConfig.hpp"

/*
Permet d'analyser et de valider ton fichier .conf pour gérer la transformation du
texte brut en objets C++, s'assurant au passage qu'il n'y a pas d'erreurs de syntaxe
(accolades manquantes, ports invalides) afin d'éviter que le serveur ne se lance
avec une mauvaise configuration.
*/

class ConfigParser
{
	public:
		// Prend le chemin du fichier et renvoie la liste des serveurs configurés
		static std::vector<ServerConfig> parse(std::string filename);
};

#endif