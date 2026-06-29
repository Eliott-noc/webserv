#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "include.hpp"

/*
Permet de décortiquer la chaîne de caractères brute envoyée par le client pour gérer
l'interprétation du protocole HTTP, en isolant précisément ce que l'utilisateur veut
faire (GET, POST, DELETE), quel fichier il cherche, et quelles informations
supplémentaires il a envoyées dans les en-têtes (headers).
*/

typedef enum	s_request_state
{
	READING_REQUEST_LINE,
	READING_HEADERS,
	READING_BODY,
	FINISHED,
	ERROR
}	t_request_state;

class Request
{
	private:
		std::string							_method;
		std::string							_path;
		std::string							_query_string;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		bool								_is_chunked;
		size_t								_content_length;
		t_request_state						_state;
		std::string							_raw_buffer;
		std::string							_tmp_file;
		int									_body_fd;
		size_t								_bytes_received;
		int									_client_fd;

	public:
		Request(int client_fd);
		Request(const Request &other);
		~Request();
		
		Request &operator=(const Request &other);

		int									parse(std::string raw_data, size_t max_body_limit);

		std::string							getMethod() const;
		std::string							getPath() const;
		std::string							getQueryString() const;
		std::map<std::string, std::string>	getHeaders() const;
		std::string							getBodyFile() const;

	private:
		void								_requestLine();
		void								_scanHeader();
		bool								_chunked(size_t max_body_limit);
};

#endif