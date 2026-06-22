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

void	Response::makeResponse(Request &req, ServerConfig &config)
{
	
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

void	Response::_handleGet(Request &req, ServerConfig &config)
{
	struct stat	s;
	std::string	full_path = config.getRoot() + req.getPath();
	
	if (stat(full_path.c_str(), &s) == 0)
	{
		if (s.st_mode &S_IFDIR)
		{
			//dossier
		}
		else if (s.st_mode &S_IFREG)
		{
			if (config.getIndex() != "")
				full_path += config.getIndex();
			else if (config.getAutoIndex() == 1)
				//liste les fichier
			else
				
		}
	}
	else 
		buildErrorPage(404, config);
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
