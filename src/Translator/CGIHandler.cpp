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

/*
 * WHAT : Pilote l'exécution du script (préparation, fork, récupération).
 * WHY  : Point d'entrée pour transformer une requête statique en exécution dynamique.
 * Gère le nettoyage de la mémoire même en cas d'échec du pipe ou du fork.
 * RETURN: "" si erreur, sinon l'output du prog execute.
*/

void CGIHandler::launch(Request &req, std::string script_path, Location loc, int &out_pipe_fd, pid_t &out_pid)
{
	char		*args[3];
	int			pipe_out[2];
	std::string	interpreter;

	_setupEnv(req, script_path);
	_convertEnvMapToArray();

	interpreter = loc.getCGIPath();
	args[0] = (char *)interpreter.c_str();
	args[1] = (char *)script_path.c_str();
	args[2] = NULL;

	if (pipe(pipe_out) == -1)
		throw std::runtime_error("pipe failed");

	pid_t pid = fork();

	if (pid == -1)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		throw std::runtime_error("fork failed");
	}
	if (pid == 0)
		_childProcess(req, args, pipe_out);

	close(pipe_out[1]);
	fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

	out_pipe_fd = pipe_out[0];
	out_pid = pid;
}

/*
 * WHAT : Remplit une map avec les variables d'environnement.
 * WHY  : C'est le seul moyen de communication avec le script externe (Python/PHP).
 * On transmet la méthode, les arguments (Query String) et les infos de taille.
*/

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

/*
 * WHAT : Convertit la std::map en tableau de pointeurs char** (format C).
 * WHY  : La fonction système execve() exige un tableau de chaînes de caractères 
 * terminé par NULL pour définir l'environnement du nouveau processus.
*/

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

/*
 * WHAT: free tableau de char**.
 * WHY: utils.
*/

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

/*
 * WHAT : Configuration du processus fils (le script).
 * WHY  : Redirige STDIN vers le fichier du Body et STDOUT vers le pipe.
 * Remplace le code du serveur par celui de l'interpréteur (Python) via execve.
*/

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
