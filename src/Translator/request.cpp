#include "../../inc/request.hpp"

#define MAX_URI_LENGTH 4096
#define MAX_HEADER_SIZE 8192

Request::Request(int client_fd) :
	_is_chunked(0),
	_keep_alive(0),
	_content_length(0),
	_state(READING_REQUEST_LINE),
	_body_fd(-1),
	_bytes_received(0),
	_client_fd(client_fd),
	_status_code(200) {}

Request::Request(const Request &other)
{
	*this = other;
}

Request::~Request() 
{
	if (_body_fd != -1)
		close(_body_fd);
	if (!_tmp_file.empty())
		std::remove(_tmp_file.c_str());
}

Request	&Request::operator=(const Request &other)
{
	if (this != &other)
	{
		if (_body_fd != -1)
			close(_body_fd);
		if (!_tmp_file.empty())
			std::remove(_tmp_file.c_str());

		_method = other._method;
		_path = other._path;
		_query_string = other._query_string;
		_version = other._version;
		_headers = other._headers;
		_is_chunked = other._is_chunked;
		_keep_alive = other._keep_alive;
		_content_length = other._content_length;
		_state = other._state;
		_tmp_file = other._tmp_file;
		_body_fd = other._body_fd;
		_bytes_received = other._bytes_received;
		_client_fd = other._client_fd;
		_status_code = other._status_code;
	}
	return *this;
}

size_t	hexToDecimal(std::string hexStr)
{
	size_t				x;
	std::stringstream	ss;
	
	ss << std::hex << hexStr;
	if (!(ss >> x))
		return 0xFFFFFFFF;
	return x;
}

void	Request::_requestLine(std::string &buffer)
{
	size_t		pos;
	size_t		i;
	std::string	first_line;
	std::string	extra;

	pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return ;

	first_line = buffer.substr(0, pos);

	if (first_line.find('\t') != std::string::npos)
	{
		_state = ERROR;
		_status_code = 400;
		return ;
	}

	std::stringstream	ss(first_line);

	if (!(ss >> _method >> _path >> _version))
	{
		_state = ERROR;
		_status_code = 400;
		return ;
	}

	if (_version == "HTTP/1.1")
		_keep_alive = true;
	else if (_version == "HTTP/1.0")
		_keep_alive = false;
	else
	{
		_state = ERROR;
		_status_code = 505;
		return ;
	}

	_path = _urlDecode(_path);

	if (_method != "GET" && _method != "POST" && _method != "DELETE")
	{
		_state = ERROR;
		_status_code = 400;
		return ;
	}

	if (ss >> extra)
	{
		_state = ERROR;
		_status_code = 400;
		return ;
	}

	if ((i = _path.find('?')) != std::string::npos)
	{
		_query_string = _path.substr(i + 1);
		_path = _path.substr(0, i);
	}

	buffer.erase(0, pos + 2);
	_state = READING_HEADERS;
}

void	Request::_scanHeader(std::string &buffer)
{
	size_t		pos;
	std::string	line;
	size_t		colon_pos;
	std::string	key;
	std::string	value;

	while ((pos = buffer.find("\r\n")) != std::string::npos)
	{
		line = buffer.substr(0, pos);

		if (line.empty())
		{
			buffer.erase(0, 2);
			_state = READING_BODY;
			return;
		}

		colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			key = line.substr(0, colon_pos);
			for (size_t j = 0; j < key.length(); ++j)
				key[j] = std::tolower(key[j]);

			value = line.substr(colon_pos + 1);
			size_t first = value.find_first_not_of(" \t");
			size_t last = value.find_last_not_of(" \t");
			if (first != std::string::npos)
				value = value.substr(first, (last - first + 1));
			else
				value = "";

			if (key == "connection")
			{
				std::string value_lower = value;
				for (size_t j = 0; j < value_lower.length(); ++j)
					value_lower[j] = std::tolower(value_lower[j]);

				if (value_lower == "keep-alive")
					_keep_alive = true;
				else if (value_lower == "close")
					_keep_alive = false;
			}

			if (key == "transfer-encoding" && value.find("chunked") != std::string::npos)
				_is_chunked = true;
			
			_headers[key] = value;
		}
		buffer.erase(0, pos + 2);
	}
}

bool	Request::_chunked(std::string &buffer, unsigned long max_body_limit)
{
	unsigned long		chunkSize;
	unsigned long		pos = buffer.find("\r\n");
	std::string			chunkData;

	if (pos == std::string::npos)
		return true;

	chunkSize = hexToDecimal(buffer.substr(0, pos));
	if (chunkSize == 0xFFFFFFFF)
	{
		_state = ERROR;
		_status_code = 400;
		return true; 
	}
	
	if (_content_length + chunkSize > max_body_limit)
	{
		_state = ERROR;
		_status_code = 413;
		return true;
	}

	if (chunkSize == 0)
	{
		if (_body_fd != -1) { close(_body_fd); _body_fd = -1; }
		_state = FINISHED;
		buffer.erase(0, pos + 4);
		return true;
	}

	if (buffer.size() < pos + 2 + chunkSize + 2)
		return true;

	if (_body_fd == -1)
	{
		std::stringstream	ss;
		ss << "/tmp/body_client_" << _client_fd << ".tmp";
		_tmp_file = ss.str();
		_body_fd = open(_tmp_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	}
	
	chunkData = buffer.substr(pos + 2, chunkSize);
	write(_body_fd, chunkData.c_str(), chunkData.size());
	
	_content_length += chunkSize;
	buffer.erase(0, pos + 2 + chunkSize + 2);
	return false;
}

int	Request::parse(std::string &buffer, unsigned long max_body_limit)
{
	while (_state != FINISHED && _state != ERROR)
	{
		if (_state == READING_REQUEST_LINE || _state == READING_HEADERS)
		{
			size_t	end_headers = buffer.find("\r\n\r\n");
			if ((end_headers == std::string::npos && buffer.size() > MAX_HEADER_SIZE) ||
				(end_headers != std::string::npos && end_headers > MAX_HEADER_SIZE))
			{
				_state = ERROR;
				_status_code = 431;
				break;
			}
		}

		if (_state == READING_REQUEST_LINE)
		{
			size_t pos = buffer.find("\r\n");
			if (pos == std::string::npos)
			{
				if (buffer.length() > MAX_URI_LENGTH) {
					_state = ERROR;
					_status_code = 414;
				}
				break;
			}
			if (pos > MAX_URI_LENGTH) {
				_state = ERROR;
				_status_code = 414;
				break;
			}
			_requestLine(buffer);
		}
		else if (_state == READING_HEADERS)
		{
			if (buffer.find("\r\n") == std::string::npos)
				break;
			_scanHeader(buffer);
			if (_state == READING_HEADERS)
				break;
		}
		else if (_state == READING_BODY)
		{
			if (_method == "POST" && !_headers.count("content-length") && !_is_chunked)
			{
				_state = ERROR;
				_status_code = 411;
				break;
			}

			if (_is_chunked)
			{
				if (_chunked(buffer, max_body_limit) == true)
					break;
			}
			else if (_headers.count("content-length"))
			{
				if (_content_length == 0)
				{
					char *endptr;
					long test_len = strtol(_headers["content-length"].c_str(), &endptr, 10);
					if (*endptr != '\0' || test_len < 0) {
						_state = ERROR;
						_status_code = 400;
						break;
					}
					_content_length = (unsigned long)test_len;
				}
				
				if (_content_length > max_body_limit)
				{
					if (_body_fd != -1) { close(_body_fd); _body_fd = -1; }
					_state = ERROR;
					_status_code = 413;
					break;
				}

				if (_body_fd == -1)
				{
					std::stringstream	ss;
					ss << "/tmp/body_client_" << _client_fd << ".tmp";
					_tmp_file = ss.str();
					_body_fd = open(_tmp_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
				}

				unsigned long	remaining = _content_length - _bytes_received;
				unsigned long	to_write = (buffer.size() < remaining) ? buffer.size() : remaining;

				if (to_write > 0)
				{
					write(_body_fd, buffer.c_str(), to_write);
					_bytes_received += to_write;
					buffer.erase(0, to_write);
				}

				if (_bytes_received >= _content_length)
				{
					if (_body_fd != -1) { close(_body_fd); _body_fd = -1; }
					_state = FINISHED;
				}
				else
					break;
			}
			else
				_state = FINISHED;
		}
	}

	if (_state == FINISHED)
	{
		if (_headers.find("host") == _headers.end() || _headers["host"].empty())
		{
			_state = ERROR;
			_status_code = 400;
			return 400;
		}
		return 200;
	}
	
	if (_state == ERROR)
		return _status_code;

	return 1;
}

std::string	Request::_urlDecode(std::string str)
{
	std::string	res;
	std::string	hex;
	char		c;

	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == '%' && i + 2 < str.length())
		{
			hex = str.substr(i + 1, 2);
			c = static_cast<char>(hexToDecimal(hex));
			res += c;
			i += 2;
		}
		else if (str[i] == '+')
			res += ' ';
		else
			res += str[i];
	}
	return res;
}

std::string	Request::getMethod() const
{
	return _method;
}

std::string	Request::getPath() const
{
	return _path;
}

std::string	Request::getQueryString() const
{
	return _query_string;
}

std::string	Request::getBodyFile() const
{
	return _tmp_file;
}

std::map<std::string, std::string>	Request::getHeaders() const
{
	return _headers;
}

bool	Request::getKeepAlive() const
{
	return _keep_alive;
}

unsigned long	Request::getContentLength() const
{
	return _content_length;
}

int		Request::getClientFd() const
{
	return _client_fd;
}

int Request::getStatusCode() const
{
	return _status_code;
}
