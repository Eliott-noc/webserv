#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "webserv.hpp"
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
		std::map<std::string, std::string> _env;
		std::string _script_path;

	public:
		CGIHandler();
		CGIHandler(const CGIHandler &src);
		~CGIHandler();

		CGIHandler &operator=(const CGIHandler &src);

		// Prépare et exécute le CGI, renvoie le résultat
		std::string execute(Request &req, std::string script_path);

	private:
		void	_setupEnv(Request &req, std::string script_path);
		char**	_getEnvAsCStyleArray() const; // Pour execve()
};

#endif