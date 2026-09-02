#include "../../inc/response.hpp"

Response::Response() :
	_status_code(0),
	_file_fd(-1),
	_file_size(0),
	_total_sent(0),
	_headers_sent(0),
	_is_finished(0),
	_is_error(0),
	_cgi_pending(false),
	_cgi_fd(-1),
	_cgi_pid(-1),
	_cgi_config(NULL) {}

Response::Response(const Response &other)
{
	*this = other;
}

Response::~Response()
{
	if (_file_fd != -1)
		close(_file_fd);
}

Response &Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_body = other._body;
		_status_code = other._status_code;
		_headers = other._headers;
		_header_buffer = other._header_buffer;
		_file_fd = other._file_fd;
		_file_size = other._file_size;
		_total_sent = other._total_sent;
		_headers_sent = other._headers_sent;
		_is_finished = other._is_finished;
		_is_error = other._is_error;
		_cgi_pending = other._cgi_pending;
		_cgi_fd = other._cgi_fd;
		_cgi_pid = other._cgi_pid;
		_cgi_buffer = other._cgi_buffer;
		_cgi_start = other._cgi_start;
		_cgi_config = other._cgi_config;
		_cgi_loc = other._cgi_loc;
	}
	return *this;
}

/*
 * WHAT : Applique la sécurité (normalizePath), cherche la 'Location' la plus précise, 
 * vérifie les méthodes autorisées, puis utilise bon handler (GET/POST/DELETE).
 * WHY : Centralise toute la logique pour faire la reponse au client et le respect de la
 * configuration de l'Architecte avant toute action sur le disque dur.
 */

void	Response::makeResponse(Request &req, ServerConfig &config)
{
	std::string	clean_path = _normalizePath(req.getPath());
	std::string	root;
	std::string	full_path;
	CGIHandler	cgi;
	std::string	cgi_output;

	if (clean_path == "ERROR")
	{
		buildErrorPage(400, config, NULL);
		return;
	}
	_headers["Date"] = getHttpDate();
	_headers["Server"] = "webserv/1.0";
	const Location	*loc = config.getLocationForPath(clean_path);
	Location		defaultLoc; 
	if (!loc)
	{
		defaultLoc.setPath("/"); 
		defaultLoc.setRoot(config.getRoot());
		
		std::vector<std::string>	serverIndexes = config.getIndex();
		for (size_t i = 0; i < serverIndexes.size(); ++i)
			defaultLoc.setIndex(serverIndexes[i]);

		defaultLoc.setAutoIndex(config.getAutoIndex());
		
		defaultLoc.addMethod("GET");

		loc = &defaultLoc;
	}

	if (loc->getReturnCode() != 0)
	{
		_status_code = loc->getReturnCode();
		_headers["Location"] = loc->getReturnUrl();
		_headers["Content-Length"] = "0";
		_generateResponse(_status_code);
		return ;
	}

	std::string	method_to_check = req.getMethod();

	if (!_isMethodAllowed(method_to_check, loc->getMethods()))
	{
		buildErrorPage(405, config, loc);
		return;
	}

	if (req.getContentLength() > loc->getClientMaxBodySize())
	{
		buildErrorPage(413, config, loc);
		return;
	}

	root = loc->getRoot().empty() ? config.getRoot() : loc->getRoot();
	full_path = root + clean_path;

	if (_isCGI(full_path, *loc))
	{
		startCGI(req, full_path, *loc, config);
		return; // rien d'autre à faire maintenant, ServerManager gère la suite
	}
	if (req.getMethod() == "GET")
		_handleGet(req, config, *loc, full_path);
	else if (req.getMethod() == "POST")
		_handlePost(req, config, *loc, full_path);
	else if (req.getMethod() == "DELETE")
		_handleDelete(config, *loc, full_path);

	// std::cout << "[DEBUG] CODE DE STATUT RENVOYÉ : " << _status_code << std::endl;
}

void Response::buildErrorPage(int code, ServerConfig &config, const Location *loc)
{
	_status_code = code;
	_headers.clear();
	_body.clear();

	std::string messageError = _getMessageError(code);

	
	_body = "<!DOCTYPE html>\n<html>\n<head>\n<title>" + messageError + "</title>\n";
	_body += "<style>\n";
	_body += "body { font-family: 'Fira Code', 'Consolas', monospace; background-color: #1e1d1b; color: #e8c37d; text-align: center; padding-top: 15vh; }\n";
	_body += "h1 { color: #f5a933; text-shadow: 0 0 10px rgba(245, 169, 51, 0.3); text-transform: uppercase; letter-spacing: 2px; }\n";
	_body += "hr { border: 0; border-top: 1px dashed #5e4d31; width: 50%; max-width: 400px; margin: 20px auto; }\n";
	_body += "p { color: #d1b890; font-size: 0.9rem; }\n";
	_body += "</style>\n</head>\n<body>\n";
	
	_body += "<h1>" + messageError + " <span class=\"cursor\"></span></h1>\n";
	_body += "<hr>\n<p>[ webserv/1.0 - system exception ]</p>\n";
	_body += "</body>\n</html>";

	if (loc != NULL)
		_checkConfig(config, loc, code);
 
	_headers["Content-Type"] = "text/html";
	std::stringstream ss_len;
	ss_len << _body.length();
	_headers["Content-Length"] = ss_len.str();

	_generateResponse(code);
}

std::string	Response::getRawResponse() const
{
	return _header_buffer + _body;
}

/*
 * Moteur d'envoi itératif (Streaming).
 * WHAT : Envoie les headers, puis le fichier par blocs de 8 Ko à chaque appel.
 * WHY : On utilise que 8ko, parce que ca permet de ne jamais surcharger la RAM, et de
 * quand meme envoyer assez rapidement la reponse aux clinets.
 */

void	Response::sendResponse(int socket_fd)
{
	int	ret;

	if (socket_fd == -1)
	{
		_headers_sent = true;
		_is_finished = true;
		return;
	}

	if (!_headers_sent)
	{
		if (_header_buffer.empty())
			return;
		ret = send(socket_fd, _header_buffer.c_str(), _header_buffer.size(), MSG_NOSIGNAL);
		if (ret <= 0){
			_is_error = true;
			_is_finished = true;
			return;
		}
		_headers_sent = true;

		if (_file_fd == -1 && _body.empty())
			_is_finished = true;
		return;
	}

	if (_file_fd == -1 && !_body.empty())
	{
		ret = send(socket_fd, _body.c_str(), _body.size(), MSG_NOSIGNAL);
		if (ret <= 0){
			_is_error = true;
			_is_finished = true;
			return;
		}
		_is_finished = true;
		_body.clear();
		return;
	}

	if (_file_fd != -1)
	{
		char	buffer[8192];
		int		bytes_read = read(_file_fd, buffer, 8192);

		if (bytes_read > 0)
		{
			ret = send(socket_fd, buffer, bytes_read, MSG_NOSIGNAL);
			if (ret <= 0){
				_is_error = true;
				_is_finished = true;
				return;
			}
			_total_sent += ret;
		}

		if (bytes_read <= 0 || _total_sent >= _file_size)
		{
			_is_finished = true;
			close(_file_fd);
			_file_fd = -1;
		}
	}
}

bool Response::isError() const
{
	return _is_error;
}

bool Response::isFinished() const
{
	return _is_finished;
}

std::string Response::getHttpDate() {
	char buffer[100];
	time_t now = time(NULL);

	struct tm* tm_info = gmtime(&now);

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);	
	return std::string(buffer);
}

bool Response::isCGIPending() const
{
	return _cgi_pending;
}

int Response::getCGIFd() const
{
	return _cgi_fd;
}

pid_t Response::getCGIPid() const
{
	return _cgi_pid;
}

void Response::startCGI(Request &req, std::string full_path, const Location &loc, ServerConfig &config)
{
	CGIHandler cgi;
	int   fd;
	pid_t pid;

	cgi.launch(req, full_path, loc, fd, pid);

	_cgi_pending = true;
	_cgi_fd = fd;
	_cgi_pid = pid;
	_cgi_buffer.clear();
	_cgi_config = &config;
	_cgi_loc = loc;
	gettimeofday(&_cgi_start, NULL);
}

void Response::feedCGIChunk(const std::string &chunk)
{
	_cgi_buffer += chunk;
}

void Response::finishCGI()
{
	if (_cgi_fd != -1)
	{
		close(_cgi_fd);
		_cgi_fd = -1;
	}
	_cgi_pending = false;

	if (_cgi_buffer.empty())
	{
		if (_cgi_config)
			buildErrorPage(500, *_cgi_config, &_cgi_loc);
		else
		{
			_status_code = 500;
			_body = "<h1>500 Internal Server Error</h1>";
			_headers.clear();
			_headers["Content-Type"] = "text/html";
			std::stringstream ss_len;
			ss_len << _body.length();
			_headers["Content-Length"] = ss_len.str();
			_generateResponse(500);
		}
		return;
	}

	_parseCGIOutput(_cgi_buffer);
	std::stringstream ss_len;
	ss_len << _body.length();
	_headers["content-length"] = ss_len.str();
	_generateResponse(200);
}

bool Response::checkCGITimeout()
{
	if (!_cgi_pending)
		return false;

	struct timeval now;
	gettimeofday(&now, NULL);
	double elapsed = (now.tv_sec - _cgi_start.tv_sec) + (now.tv_usec - _cgi_start.tv_usec) / 1000000.0;

	return (elapsed > 3.0);
}

void Response::abortCGI()
{
	_cgi_pending = false;
	_cgi_fd = -1;
	_cgi_pid = -1;
	_cgi_buffer.clear();
}
