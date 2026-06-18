#include "../../inc/request.hpp"

Request::Request() : _is_chunked(0), _content_length(0) {}

Request::Request(const Request &other)
{
	*this = other;
}

Request::~Request() {}

Request	&Request::operator=(const Request &other)
{
	if (this != &other)
	{
		_method = other._method;
		_path = other._path;
		_query_string = other._query_string;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
		_is_chunked = other._is_chunked;
		_content_length = other._content_length;
	}
	return *this;
}

void	Request::requestLine(std::string &raw_data)
{
	size_t		pos;
	size_t		i;
	std::string	first_line;
	
	pos = raw_data.find("\r\n");
	if (pos == std::string::npos)
		return;

	first_line = raw_data.substr(0, pos);

	std::stringstream	ss(first_line);
	ss >> _method;
	ss >> _path;
	ss >> _version;
	if ((i = _path.find('?')) != std::string::npos)
	{
		_query_string = _path.substr(i + 1);
		_path = _path.substr(0, i);
	}
	raw_data.erase(0, pos + 2);
}

void	Request::scanHeader(std::string &raw_data)
{
	size_t		pos;
	size_t		colon_pos;
	std::string	line;
	std::string	key;
	std::string	value;
	size_t		first;

	while (1)
	{
		pos = raw_data.find("\r\n");
		if (pos == std::string::npos)
			break;

		line = raw_data.substr(0, pos);
		
		if (line.empty()) 
			break;

		colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			key = line.substr(0, colon_pos);
			value = line.substr(colon_pos + 1);
			first = value.find_first_not_of(" ");

			if (first != std::string::npos)
				value = value.substr(first);

			_headers[key] = value;
		}
		raw_data.erase(0, pos + 2);
	}
}

int	Request::parse(std::string raw_data, size_t max_body_limit)
{
	requestLine(raw_data);
	scanHeader(raw_data);

	if (raw_data.substr(0, 2) == "\r\n")
		raw_data.erase(0, 2);

	if (_headers.count("Content-Length"))
	{
		long len = atol(_headers["Content-Length"].c_str());
		
		if (static_cast<size_t>(len) > max_body_limit) {
			return 413;
		}
		_content_length = static_cast<size_t>(len);

		if (raw_data.size() >= _content_length)
			_body = raw_data.substr(0, _content_length);
		else
			return 400;
	}

	std::cout << "Methode:        " << _method << std::endl;
	std::cout << "path:           " << _path << std::endl;
	std::cout << "Query String:   " << _query_string << std::endl;
	std::cout << "Vesrion:        " << _version << std::endl;
	std::cout << "Host:           " << _headers["Host"] << std::endl;
	std::cout << "Content-Length: " << _headers["Content-Length"] << std::endl;
	std::cout << "Body            " << _body << std::endl;
	std::cout << "Is Chunked:     " << _is_chunked << std::endl;
	std::cout << "Content Length: " << _content_length << std::endl;

	return 200;
}
