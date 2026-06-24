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
		std::string							_header_buffer;
		int									_file_fd;
		size_t								_file_size;
		size_t								_total_sent;
		bool								_headers_sent;
		bool								_is_finished;

	public:
		Response();
		Response(const Response &src);
		~Response();
		
		Response &operator=(const Response &src);

		void		makeResponse(Request &req, ServerConfig &config);
		void		buildErrorPage(int code, ServerConfig &config);
		std::string	getRawResponse() const;
		void		sendResponse(int client_socket);
		bool		isFinished() const; 

	private:
		bool		_isMethodAllowed(std::string method, std::vector<std::string> const &allowedMethods);
		void		_handleGet(Request &req, ServerConfig &config, const Location &loc, std::string full_path);
		void		_handlePost(Request &req, ServerConfig &config, const Location &loc, std::string full_path);
		void		_handleDelete(ServerConfig &config, std::string full_path);
		std::string	_getMimeType(std::string path);
		std::string	_getStatusMessage(int code);
		int			_checkConfig(ServerConfig &config, int code);
		std::string	_getMessageError(int code);
		void		_generateResponse(int code);
		std::string	_generateAutoIndex(std::string full_path, std::string request_path);
};

#endif