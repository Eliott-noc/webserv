#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "include.hpp"
# include "request.hpp"
# include "serverConfig.hpp"
# include "CGIHandler.hpp"

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
	bool								_is_error;

	bool								_cgi_pending;
	int									_cgi_fd;
	pid_t								_cgi_pid;
	std::string							_cgi_buffer;
	struct timeval						_cgi_start;
	ServerConfig						*_cgi_config;
	Location							_cgi_loc;

public:
	Response();
	Response(const Response &src);
	~Response();
	Response &operator=(const Response &src);

	void		makeResponse(Request &req, ServerConfig &config);
	void		buildErrorPage(int code, ServerConfig &config, const Location *loc);
	std::string	getRawResponse() const;
	void		sendResponse(int client_socket);
	bool		isFinished() const;
	bool		isError() const;

	bool		isCGIPending() const;
	int			getCGIFd() const;
	pid_t		getCGIPid() const;
	void		startCGI(Request &req, std::string full_path, const Location &loc, ServerConfig &config);
	void		feedCGIChunk(const std::string &chunk);
	void		finishCGI();
	bool		checkCGITimeout();
	void		abortCGI();

private:
	bool		_isMethodAllowed(std::string method, std::vector<std::string> const &allowedMethods);
	void		_handleGet(Request &req, ServerConfig &config, const Location &loc, std::string full_path);
	void		_handlePost(Request &req, ServerConfig &config, const Location &loc, std::string full_path);
	void		_handleDelete(ServerConfig &config, const Location &loc, std::string full_path);
	std::string	_getMimeType(std::string path);
	std::string	_getStatusMessage(int code);
	bool		_checkConfig(ServerConfig &config, const Location *loc, int code);
	std::string	_getMessageError(int code);
	void		_generateResponse(int code);
	std::string	_generateAutoIndex(std::string full_path, std::string request_path);
	bool		_isCGI(std::string const &path, const Location &loc);
	std::string	_normalizePath(std::string path);
	void		_parseCGIOutput(std::string &cgi_output);
	std::string	getHttpDate();
};

#endif