#include "../../inc/request.hpp"

Request::Request(int client_fd) : _is_chunked(0), _content_length(0) , _state(READING_REQUEST_LINE), _body_fd(-1), _bytes_received(0), _client_fd(client_fd) {}

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
		_method = other._method;
		_path = other._path;
		_query_string = other._query_string;
		_version = other._version;
		_headers = other._headers;
		_is_chunked = other._is_chunked;
		_content_length = other._content_length;
		_state = other._state;
		_raw_buffer = other._raw_buffer;
		_tmp_file = other._tmp_file;
		_body_fd = other._body_fd;
		_bytes_received = other._bytes_received;
		_client_fd = other._client_fd;
	}
	return *this;
}

size_t hexToDecimal(std::string hexStr)
{
	size_t				x;
	std::stringstream	ss;

	ss << std::hex << hexStr;
	ss >> x;

	return x;
}

void	Request::_requestLine()
{
	size_t		pos;
	size_t		i;
	std::string	first_line;
	
	pos = _raw_buffer.find("\r\n");
	if (pos == std::string::npos)
		return ;

	first_line = _raw_buffer.substr(0, pos);

	std::stringstream	ss(first_line);
	if (!(ss >> _method >> _path >> _version))
	{
		_state = ERROR;
		return ;
	}
	std::string	extra;
	if (ss >> extra)
	{
		_state = ERROR;
		return ;
	}

	if ((i = _path.find('?')) != std::string::npos)
	{
		_query_string = _path.substr(i + 1);
		_path = _path.substr(0, i);
	}
	_raw_buffer.erase(0, pos + 2);
	_state = READING_HEADERS;
}

void Request::_scanHeader()
{
	size_t		pos;
	std::string	line;
	size_t		colon_pos;
	std::string	key;
	std::string	value;
	size_t		first;

	while ((pos = _raw_buffer.find("\r\n")) != std::string::npos)
	{
		line = _raw_buffer.substr(0, pos);
		
		if (line.empty())
		{
			_raw_buffer.erase(0, 2);
			_state = READING_BODY;
			return	;
		}

		colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			key = line.substr(0, colon_pos);
			value = line.substr(colon_pos + 1);

			if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos)
				_is_chunked = true;

			first = value.find_first_not_of(" ");
			if (first != std::string::npos)
				value = value.substr(first);
			
			_headers[key] = value;
		}

		_raw_buffer.erase(0, pos + 2);
	}
}

bool Request::_chunked(size_t max_body_limit)
{
	size_t chunkSize;
	size_t pos = _raw_buffer.find("\r\n");

	if (pos == std::string::npos)
		return true;

	chunkSize = hexToDecimal(_raw_buffer.substr(0, pos));
	
	if (_content_length + chunkSize > max_body_limit)
	{
		_state = ERROR;
		return true;
	}

	if (chunkSize == 0)
	{
		if (_body_fd != -1)
		{
			close(_body_fd);
			_body_fd = -1;
		}
		_state = FINISHED;
		_raw_buffer.erase(0, pos + 4);
		return true;
	}

	if (_raw_buffer.size() < pos + 2 + chunkSize + 2)
		return true;

	if (_body_fd == -1)
	{
		std::stringstream	ss;
		ss << "/tmp/body_client_" << _client_fd << ".tmp";
		_tmp_file = ss.str();
		_body_fd = open(_tmp_file.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, 0644);
	}
	
	std::string chunkData = _raw_buffer.substr(pos + 2, chunkSize);
	
	write(_body_fd, chunkData.c_str(), chunkData.size());
	
	_content_length += chunkSize;
	_raw_buffer.erase(0, pos + 2 + chunkSize + 2);
	return false;
}

int Request::parse(std::string chunk, size_t max_body_limit)
{
	_raw_buffer += chunk;

	if (_state != READING_BODY && _raw_buffer.size() > 8192)
	{
		_state = ERROR;
		return 431;
	}

	while (_state != FINISHED && _state != ERROR)
	{
		if (_state == READING_REQUEST_LINE)
		{
			if (_raw_buffer.find("\r\n") == std::string::npos)
				break ;
			_requestLine();
		}
		else if (_state == READING_HEADERS)
		{
			if (_raw_buffer.find("\r\n") == std::string::npos)
				break;

			_scanHeader();

			if (_state == READING_HEADERS)
				break;
		}
		else if (_state == READING_BODY)
		{
			if (_method == "POST" && !_headers.count("Content-Length") && !_is_chunked)
			{
				_state = ERROR;
				return 411;
			}

			if (_is_chunked)
			{
				if (_chunked(max_body_limit) == true)
					break;
			}
			else if (_headers.count("Content-Length"))
			{
				if (_content_length == 0)
					_content_length = atol(_headers["Content-Length"].c_str());

				if (_content_length > max_body_limit)
				{
					_state = ERROR;
					return 413;
				}

				if (_body_fd == -1)
				{
					std::stringstream ss;
					ss << "/tmp/body_client_" << _client_fd << ".tmp";
					_tmp_file = ss.str();
					_body_fd = open(_tmp_file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
				}

				size_t to_write = _raw_buffer.size();
				if (to_write > 0)
				{
					write(_body_fd, _raw_buffer.c_str(), to_write);
					_bytes_received += to_write;
					_raw_buffer.clear();
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
			{
				_state = FINISHED;
			}
		}
	}

	if (_state == FINISHED)
	{
		if (_headers.find("Host") == _headers.end())
		{
			_state = ERROR;
			return 400;
		}
		return 200;
	}
	
	if (_state == ERROR) return 400;

	return 1;
}

//Transfer-Encoding: chunked au lieu de Content-Length

std::string	Request::getMethod() const
{
	return _method;
}

std::string	Request::getPath() const
{
	return _path;
}

std::string	Request::getBodyFile() const
{
	return _tmp_file;
}

std::map<std::string, std::string>	Request::getHeaders() const
{
	return _headers;
}
