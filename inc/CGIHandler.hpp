#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "include.hpp"
# include "request.hpp"

/*
Permet d'exécuter des programmes externes (Python, PHP, etc.) pour gérer le contenu
dynamique du site, en créant un environnement sécurisé pour lancer le script et en
récupérant son résultat pour le redonner à la réponse, sans que l'exécution du script
ne ralentisse les autres clients.
*/

class CGIHandler
{
	private:
		std::map<std::string, std::string>	_env;
		char**								_envArray;
		std::string							_scriptPath;

	public:
		CGIHandler();
		CGIHandler(const CGIHandler &other);
		~CGIHandler();

		CGIHandler	&operator=(const CGIHandler &other);

		std::string	execute(Request &req, std::string script_path);

	private:
		void	_setupEnv(Request &req, std::string script_path);
		void	_convertEnvMapToArray();
		void	_freeEnvArray();
};

#endif