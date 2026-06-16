#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "webserv.hpp"
# include "request.hpp"

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