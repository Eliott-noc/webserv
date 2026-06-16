#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "webserv.hpp"

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
		Request(const Request &src);
		~Request();
		
		Request &operator=(const Request &src);

		void								parse(std::string raw_data);
		std::string							getMethod() const;
		std::string							getPath() const;
		std::string							getBody() const;
		std::map<std::string, std::string>	getHeaders() const;
};

#endif