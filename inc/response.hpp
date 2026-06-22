#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "include.hpp"
# include "request.hpp"
# include "serverConfig.hpp"

/*
Permet de fabriquer le message de réponse HTTP final pour gérer l'envoi des
ressources au client, en décidant si elle doit envoyer le contenu d'un fichier
(200 OK), une erreur personnalisée (404 Not Found), ou le résultat d'un calcul
dynamique, tout en formatant le texte selon les normes du web.
*/

class Response
{
	private:
		std::string							_response;
		std::string							_body;
		int									_status_code;
		std::map<std::string, std::string>	_headers;

	public:
		Response();
		Response(const Response &src);
		~Response();
		
		Response &operator=(const Response &src);

		void		makeResponse(Request &req, ServerConfig &config);
		void		buildErrorPage(int code, ServerConfig &config);
		std::string	getRawResponse() const;

	private:
		void		_handleGet(Request &req, ServerConfig &config);
		void		_handlePost(Request &req, ServerConfig &config);
		void		_handleDelete(Request &req, ServerConfig &config);
		std::string	_getMimeType(std::string path);
		std::string	_getStatusMessage(int code);
		int			_checkConfig(ServerConfig &config, int code);
		std::string	_getMessageError(int code);
		void		_generateResponse(int code);
};

#endif