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

std::string	CGIHandler::execute(Request &req, std::string script_path, Location loc)
{
	char	*args[3];
	int		pipe_out[2];
	int		pid;

	_setupEnv(req, script_path);
	_convertEnvMapToArray();

	std::string interpreter = loc.getCGIPath();
	
	args[0] = (char *)interpreter.c_str(); 
	args[1] = (char *)script_path.c_str();
	args[2] = NULL;

	if (pipe(pipe_out) == -1)
		return (_freeEnvArray(), "");

	pid = fork();
	if (pid == -1)
		return (close(pipe_out[0]), close(pipe_out[1]), _freeEnvArray(), "");

	if (pid == 0)
		_childProcess(req, args, pipe_out);
	return _parentProcess(pipe_out, pid);
}

void	CGIHandler::_setupEnv(Request &req, std::string script_path)
{
	std::map<std::string, std::string>	headers;


	_env.clear();
	_env["REQUEST_METHOD"] = req.getMethod();
	_env["QUERY_STRING"] = req.getQueryString();
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["PATH_TRANSLATED"] = script_path;
	
	if (req.getMethod() == "POST")
	{
		headers = req.getHeaders();
		if (headers.count("content-length"))
			_env["CONTENT_LENGTH"] = headers["content-length"];
		if (headers.count("content-type"))
			_env["CONTENT_TYPE"] = headers["content-type"];
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

void	CGIHandler::_childProcess(Request &req, char *args[3], int pipe_out[2])
{
	int	fd_in;
	int	dev_null;

	if (req.getMethod() == "POST")
	{
		fd_in = open(req.getBodyFile().c_str(), O_RDONLY);
		if (fd_in != -1)
		{
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}
	}
	else
	{
		dev_null = open("/dev/null", O_RDONLY);
		dup2(dev_null, STDIN_FILENO);
		close(dev_null);
	}

	dup2(pipe_out[1], STDOUT_FILENO);
	close(pipe_out[0]);
	close(pipe_out[1]);

	std::cerr << "[DEBUG CGI] Lancement de : " << args[0] << " avec le script : " << args[1] << std::endl;
	execve(args[0], args, _envArray);
	perror("execve failed");
	exit(1);
}

std::string CGIHandler::_parentProcess(int pipe_out[2], int pid)
{
	std::string		result;
	char			buffer[4096];
	int				status;
	struct timeval	start_time, current_time;
	
	close(pipe_out[1]);
	
	fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

	gettimeofday(&start_time, NULL);

	bool	timed_out = false;

	while (true)
	{
		int	wait_ret = waitpid(pid, &status, WNOHANG);
		
		if (wait_ret == pid)
			break;
		
		gettimeofday(&current_time, NULL);

		long	elapsed = (current_time.tv_sec - start_time.tv_sec);

		if (elapsed > 2)
		{
			timed_out = true;
			kill(pid, SIGKILL);
			break;
		}

		int	bytes_read = read(pipe_out[0], buffer, 4096);
		if (bytes_read > 0)
			result.append(buffer, bytes_read);

		usleep(10000);
	}

	int	bytes_read;
	while ((bytes_read = read(pipe_out[0], buffer, 4096)) > 0)
		result.append(buffer, bytes_read);

	close(pipe_out[0]);

	if (timed_out)
	{
		std::cerr << "[CGI] Timeout reached, process killed." << std::endl;
		return "timeout";
	}

	if (WIFSIGNALED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0))
		return "";

	return result;
}
