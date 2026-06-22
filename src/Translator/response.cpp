#include "../../inc/response.hpp"

Response::Response() : _status_code(0) {}

Response::Response(const Response &other)
{
	*this = other;
}

Response::~Response() {}

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_response = other._response;
		_body = other._body;
		_status_code = other._status_code;
		_headers = other._headers;
	}
	return *this;
}

bool	Response::_isMethodAllowed(std::string method, std::vector<std::string> const &allowedMethods)
{
	if (allowedMethods.empty())
		return true; 

	for (size_t i = 0; i < allowedMethods.size(); ++i)
	{
		if (method == allowedMethods[i])
			return true;
	}
	return false;
}

void Response::makeResponse(Request &req, ServerConfig &config)
{
	const Location	*loc = config.getLocationForPath(req.getPath());

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

	std::string	root = loc->getRoot().empty() ? config.getRoot() : loc->getRoot();
	std::string	full_path = root + req.getPath();

	if (req.getMethod() == "GET")
		_handleGet(req, config, *loc, full_path);
	// ...
}

int	Response::_checkConfig(ServerConfig &config, int code)
{
	std::map<int, std::string>	errorPages = config.getErrorPages();
	
	if (errorPages.count(code))
	{
		std::string		path = config.getRoot() + errorPages[code];
		std::ifstream	file(path.c_str(), std::ios::binary);

		if (file.is_open())
		{
			std::stringstream ss;
			ss << file.rdbuf();
			_body = ss.str();
			file.close();
			return 1;
		}
	}
	return 0;
}

std::string	Response::_getMessageError(int code)
{
	std::string		messageError = "";
	std::vector<int>	value;
	int					dupCode = code;

	while (dupCode > 0)
	{
		value.push_back(dupCode % 10);
		dupCode /= 10;
	}
	for (std::vector<int>::const_reverse_iterator i = value.rbegin(); i != value.rend(); i++)
		messageError += static_cast<char>('0' + *i);

	messageError += " " + _getStatusMessage(code);
	return messageError;
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

	std::stringstream ss_len;
	ss_len << _body.length();
	
	_headers["Content-Type"] = "text/html";
	_headers["Content-Length"] = ss_len.str();
	_headers["Server"] = "webserv/1.0";
	_response = "HTTP/1.1 " + messageError + "\r\n";

	std::map<std::string, std::string>::iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		_response += it->first + ": " + it->second + "\r\n";
	}

	_response += "\r\n";
	_response += _body;

	std::cout << "[Response] Error " << code << " generated." << std::endl;
}

std::string	Response::getRawResponse() const
{
	return _response;
}

void	Response::_handleGet(Request &req, ServerConfig &config, const Location &loc, std::string full_path)
{
	struct stat s;

	if (stat(full_path.c_str(), &s) != 0)
	{
		buildErrorPage(404, config);
		return;
	}

	if (S_ISDIR(s.st_mode))
	{
		if (!loc.getIndex().empty())
		{
			std::string indexPath = full_path;
			if (indexPath.at(indexPath.length() - 1) != '/')
				indexPath += "/";
			indexPath += loc.getIndex();

			struct stat s_index;
			if (stat(indexPath.c_str(), &s_index) == 0 && S_ISREG(s_index.st_mode))
			{
				full_path = indexPath;
				s = s_index;
			}
			else if (loc.getAutoIndex())
			{
				_body = "<html><body><h1>Index of " + req.getPath() + "</h1><hr><pre>Autoindex actif (en attente de code)</pre></body></html>";
				_headers["Content-Type"] = "text/html";
				std::stringstream ss_len; ss_len << _body.length();
				_headers["Content-Length"] = ss_len.str();
				_generateResponse(200);
				return;
			}
			else
			{
				buildErrorPage(403, config);
				return;
			}
		}
		else if (loc.getAutoIndex())
		{
			_body = "<html><body><h1>Index of " + req.getPath() + "</h1></body></html>";
			_headers["Content-Type"] = "text/html";
			std::stringstream ss_len; ss_len << _body.length();
			_headers["Content-Length"] = ss_len.str();
			_generateResponse(200);
			return;
		}
		else
		{
			buildErrorPage(403, config);
			return;
		}
	}

	if (S_ISREG(s.st_mode))
	{
		std::ifstream file(full_path.c_str(), std::ios::binary);
		if (file.is_open())
		{
			std::stringstream ss_file;
			ss_file << file.rdbuf();
			_body = ss_file.str();
			file.close();

			_headers["Content-Type"] = _getMimeType(full_path);
			std::stringstream ss_len;
			ss_len << _body.length();
			_headers["Content-Length"] = ss_len.str();

			_generateResponse(200);
		}
		else
			buildErrorPage(403, config);
	}
	else
		buildErrorPage(404, config);
}

void	Response::_handlePost(Request &req, ServerConfig &config, std::string full_path)
{
	(void)req;
	(void)config;
	(void)full_path;
}

void	Response::_handleDelete(Request &req, ServerConfig &config, std::string full_path)
{
	(void)req;
	(void)config;
	(void)full_path;
}

std::string Response::_getMimeType(std::string path)
{
	static std::map<std::string, std::string>	mimeTypes;
	size_t										pos;
	std::string									ext;

	if (mimeTypes.empty())
	{
		mimeTypes[".html"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".txt"] = "text/plain";
	}

	pos = path.find_last_of('.');
	
	if (pos == std::string::npos)
		return "application/octet-stream";

	ext = path.substr(pos);

	if (mimeTypes.count(ext))
		return mimeTypes[ext];

	return "application/octet-stream";
}

std::string Response::_getStatusMessage(int code)
{
	static std::map<int, std::string>	messages;

	if (messages.empty())
	{
		messages[200] = "OK";
		messages[201] = "Created";
		messages[204] = "No Content";
		messages[400] = "Bad Request";
		messages[403] = "Forbidden";
		messages[404] = "Not Found";
		messages[405] = "Method Not Allowed";
		messages[413] = "Payload Too Large";
		messages[500] = "Internal Server Error";
	}
	
	if (messages.count(code))
		return messages[code];
	return "Unknown Error";
}

void	Response::_generateResponse(int code)
{
	std::stringstream ss;

	_status_code = code;

	ss << "HTTP/1.1 " << _status_code << " " << _getStatusMessage(_status_code) << "\r\n";

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		ss << it->first << ": " << it->second << "\r\n";

	ss << "\r\n";

	_response = ss.str() + _body;
	
	std::cout << "[Response] Status " << _status_code << " generated." << std::endl;
}
