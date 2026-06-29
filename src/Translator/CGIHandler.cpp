#include "../../inc/CGIHandler.hpp"

CGIHandler::CGIHandler() : _envArray(NULL) {}

CGIHandler::CGIHandler(const CGIHandler &other)
{
	*this = other;
}

CGIHandler::~CGIHandler() 
{
	_freeEnvArray();
}

CGIHandler	&CGIHandler::operator=(const CGIHandler &other)
{
	_env = other._env;
	_envArray = other._envArray;
	_scriptPath = other._scriptPath;
	return *this;
}

std::string	CGIHandler::execute(Request &req, std::string script_path)
{
	int	fd[2];
	

}

void	CGIHandler::_setupEnv(Request &req, std::string script_path)
{
	_env["REQUEST_METHOD"] = req.getMethod();
	_env["QUERY_STRING"] = req.getQueryString();
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["PATH_TRANSLATED"] = script_path;
	
	if (req.getMethod() == "POST")
	{
		std::map<std::string, std::string> headers = req.getHeaders();
		if (headers.count("Content-Length"))
			_env["CONTENT_LENGTH"] = headers["Content-Length"];
		if (headers.count("Content-Type"))
			_env["CONTENT_TYPE"] = headers["Content-Type"];
	}
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = script_path;
}

void	CGIHandler::_convertEnvMapToArray()
{
	std::string	element;
	size_t		i = 0;
	
	_envArray = new char*[_env.size() + 1];

	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
	{
		element = it->first + "=" + it->second;
		
		_envArray[i] = new char[element.size() + 1];
		
		for (size_t j = 0; j < element.size(); ++j)
			_envArray[i][j] = element[j];
		
		_envArray[i][element.size()] = '\0';
		i++;
	}
	_envArray[i] = NULL;
}

void	CGIHandler::_freeEnvArray()
{
	if (_envArray)
	{
		for (int i = 0; _envArray[i] != NULL; i++)
			delete[] _envArray[i];
		delete[] _envArray;
		_envArray = NULL;
	}
}
