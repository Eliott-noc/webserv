#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "include.hpp"

/*
Permet de décortiquer la chaîne de caractères brute envoyée par le client pour gérer
l'interprétation du protocole HTTP, en isolant précisément ce que l'utilisateur veut
faire (GET, POST, DELETE), quel fichier il cherche, et quelles informations
supplémentaires il a envoyées dans les en-têtes (headers).
*/

class Request
{
	private:
		std::string							_method;        // GET, POST, DELETE
		std::string							_path;          // /index.html
		std::string							_query_string;  // ce qu'il y a après le '?' dans l'URL
		std::string							_version;       // HTTP/1.1
		std::map<std::string, std::string>	_headers;       // Host, Content-Type, etc.
		std::string							_body;
		bool								_is_chunked;
		size_t								_content_length;

	public:
		Request();
		Request(const Request &other);
		~Request();
		
		Request &operator=(const Request &other);

		int									parse(std::string raw_data, size_t max_body_limit);
		void								requestLine(std::string &raw_data);
		void								scanHeader(std::string &raw_data);
		std::string							getMethod() const;
		std::string							getPath() const;
		std::string							getBody() const;
		std::map<std::string, std::string>	getHeaders() const;
};

#endif