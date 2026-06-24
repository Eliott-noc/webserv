#include "../../inc/response.hpp"

bool	Response::_isMethodAllowed(std::string method, std::vector<std::string> const &allowedMethods)
{
	if (allowedMethods.empty())
		return true; 

	for (size_t i = 0; i < allowedMethods.size(); ++i)
		if (method == allowedMethods[i])
			return true;
	return false;
}

int	Response::_checkConfig(ServerConfig &config, int code)
{
	std::string					path;
	std::map<int, std::string>	errorPages = config.getErrorPages();
	
	if (errorPages.count(code))
	{
		path = config.getRoot() + errorPages[code];
		std::ifstream	file(path.c_str(), std::ios::binary);

		if (file.is_open())
		{
			std::stringstream	ss;

			ss << file.rdbuf();
			_body = ss.str();

			file.close();
			return 1;
		}
	}
	return 0;
}

std::string Response::_getMessageError(int code)
{
	std::stringstream ss;
	ss << code << " " << _getStatusMessage(code);
	return ss.str(); 
}

void	Response::_handleGet(Request &req, ServerConfig &config, const Location &loc, std::string full_path)
{
	struct stat	s;
	std::string	indexPath;

	if (stat(full_path.c_str(), &s) != 0)
	{
		buildErrorPage(404, config);
		return;
	}

	if (S_ISDIR(s.st_mode))
	{
		if (!loc.getIndex().empty())
		{
			indexPath = full_path;
			if (indexPath.at(indexPath.length() - 1) != '/')
				indexPath += "/";
			indexPath += loc.getIndex();

			struct stat	s_index;

			if (stat(indexPath.c_str(), &s_index) == 0 && S_ISREG(s_index.st_mode))
			{
				full_path = indexPath;
				s = s_index;
			}
			else if (loc.getAutoIndex())
			{
				_body = _generateAutoIndex(full_path, req.getPath());
				_headers["Content-Type"] = "text/html";
				
				std::stringstream	ss_len;
				
				ss_len << _body.length();
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
			_body = _generateAutoIndex(full_path, req.getPath());
			_headers["Content-Type"] = "text/html";
			
			std::stringstream	ss_len;

			ss_len << _body.length();
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
		_file_fd = open(full_path.c_str(), O_RDONLY);
		if (_file_fd == -1) {
			buildErrorPage(403, config);
			return;
		}
		_file_size = s.st_size;

		_headers["Content-Type"] = _getMimeType(full_path);
		std::stringstream ss_len;
		ss_len << _file_size;
		_headers["Content-Length"] = ss_len.str();

		_generateResponse(200);
	}
	else
		buildErrorPage(404, config);
}

void	Response::_handlePost(Request &req, ServerConfig &config, const Location &loc, std::string full_path)
{
	std::string uploadDir = loc.getUploadStore();
	if (uploadDir.empty()) { buildErrorPage(403, config); return; }

	std::string fileName = full_path.substr(full_path.find_last_of('/') + 1);
	std::string savePath = uploadDir + "/" + fileName;

	if (std::rename(req.getBodyFile().c_str(), savePath.c_str()) != 0)
	{
		std::ifstream src(req.getBodyFile().c_str(), std::ios::binary);
		std::ofstream dst(savePath.c_str(), std::ios::binary);
		if (!src.is_open() || !dst.is_open()) {
			buildErrorPage(500, config);
			return;
		}
		dst << src.rdbuf();
		src.close();
		dst.close();
		std::remove(req.getBodyFile().c_str());
	}
	_body = "<h1>Fichier cree avec succes !</h1>";
	_headers["Content-Type"] = "text/html";
	_generateResponse(201);
}

void	Response::_handleDelete(ServerConfig &config, std::string full_path)
{
	struct stat	s;

	if (stat(full_path.c_str(), &s) != 0)
	{
		buildErrorPage(404, config);
		return;
	}
	if (S_ISDIR(s.st_mode))
	{
		buildErrorPage(403, config);
		return;
	}

	if (std::remove(full_path.c_str()) == 0)
	{
		_body.clear();
		_headers.clear();
		_generateResponse(204);
	}
	else
		buildErrorPage(500, config);
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
	_status_code = code;
	std::stringstream ss;

	ss << "HTTP/1.1 " << _status_code << " " << _getStatusMessage(_status_code) << "\r\n";
	
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		ss << it->first << ": " << it->second << "\r\n";

	ss << "\r\n";
	_header_buffer = ss.str();
}

std::string	Response::_generateAutoIndex(std::string full_path, std::string request_path)
{
	std::string	html;
	std::string	name;

	DIR	*dir = opendir(full_path.c_str());
	if (!dir)
		return "";

	html = "<html><head><title>Index of " + request_path + "</title></head>";
	html += "<body><h1>Index of " + request_path + "</h1><hr><ul>";

	struct dirent	*entry;
	while ((entry = readdir(dir)) != NULL)
	{
		name = entry->d_name;

		if (name == ".")
			continue;

		html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
	}

	html += "</ul><hr></body></html>";

	closedir(dir);
	return html;
}
