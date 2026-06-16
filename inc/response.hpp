#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserv.hpp"
# include "request.hpp"
# include "serverConfig.hpp"

class Response
{
	private:
		std::string							_raw_response; // La string finale prête pour send()
		std::string							_body;
		int									_status_code;
		std::map<std::string, std::string>	_headers;

	public:
		Response();
		Response(const Response &src);
		~Response();
		
		Response &operator=(const Response &src);

		// Génère la réponse en fonction de la requête et de la config
		void		makeResponse(Request &req, ServerConfig &config);
		void		buildErrorPage(int code, ServerConfig &config);
		std::string	getRawResponse() const;

	private:
		// Fonctions d'aide internes
		void		_handleGet(Request &req, ServerConfig &config);
		void		_handlePost(Request &req, ServerConfig &config);
		void		_handleDelete(Request &req, ServerConfig &config);
		std::string	_getMimeType(std::string path);
};

#endif