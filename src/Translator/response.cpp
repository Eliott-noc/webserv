#include "../../inc/response.hpp"

Response::Response() :
	_status_code(0),
	_file_fd(-1),
	_file_size(0),
	_total_sent(0),
	_headers_sent(0),
	_is_finished(0) {}

Response::Response(const Response &other)
{
	*this = other;
}

Response::~Response()
{
	if (_file_fd != -1)
		close(_file_fd);
}

Response	&Response::operator=(const Response &other)
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
	}
	return *this;
}

void	Response::makeResponse(Request &req, ServerConfig &config)
{
	std::string		root;
	std::string		full_path;
	std::string		clean_path = normalizePath(req.getPath());
	
	if (clean_path == "ERROR")
	{
		buildErrorPage(400, config);
		return;
	}
	
	const Location	*loc = config.getLocationForPath(clean_path);

	if (!loc)
	{
		buildErrorPage(404, config);
		return;
	}

	if (!_isMethodAllowed(req.getMethod(), loc->getMethods()))
	{
		buildErrorPage(405, config);
		return;
	}

	if (loc->getRoot().empty())
		root = config.getRoot();
	else
		root = loc->getRoot();
	full_path = root + clean_path;

	if (req.getMethod() == "GET")
		_handleGet(req, config, *loc, full_path);
	if (req.getMethod() == "POST")
		_handlePost(req, config, *loc, full_path);
	if (req.getMethod() == "DELETE")
		_handleDelete(config, full_path);
}

void	Response::buildErrorPage(int code, ServerConfig &config)
{
	std::string	messageError;

	_status_code = code;
	_headers.clear();

	messageError = _getMessageError(code);

	if (!_checkConfig(config, code))
	{
		_body = "<html><head><title>" + messageError + "</title></head>";
		_body += "<body><center><h1>" + messageError + "</h1></center>";
		_body += "<hr><center>webserv/1.0</center></body></html>";
	}

	std::stringstream	ss_len;

	ss_len << _body.length();
	
	_headers["Content-Type"] = "text/html";
	_headers["Content-Length"] = ss_len.str();
	_headers["Server"] = "webserv/1.0";
	_generateResponse(code);
}

std::string	Response::getRawResponse() const
{
	return _header_buffer + _body;
}

void	Response::sendResponse(int socket_fd)
{
	int	ret;

	if (!_headers_sent)
	{
		if (_header_buffer.empty())
			return;
		ret = send(socket_fd, _header_buffer.c_str(), _header_buffer.size(), 0);
		if (ret <= 0)
			return;
		_headers_sent = true;
		
		if (_file_fd == -1 && _body.empty())
			_is_finished = true;
		return;
	}

	if (_file_fd == -1 && !_body.empty())
	{
		ret = send(socket_fd, _body.c_str(), _body.size(), 0);
		if (ret < 0)
			return;
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
			ret = send(socket_fd, buffer, bytes_read, 0);
			if (ret > 0)
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

bool Response::isFinished() const
{
	return _is_finished;
}
