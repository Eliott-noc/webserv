#include "../../inc/locArgs.hpp"
#include "../../inc/include.hpp"

void	setArgPath(Location &location, const std::string &path)
{
	std::cout << "path detected" << std::endl;

	if (path[0] != '/')
		throw std::runtime_error("Error : location path need to start with '/'");
	location.setPath(path);
}
void	setArgRoot(Location &location, const std::vector<std::string> &args)
{
	size_t	nbArgs = 0;
	
	for (size_t i = 0; i < args.size(); i++)
		nbArgs++;
	if (nbArgs < 2)
		throw std::runtime_error("Error: root must have one argument");
	else if (nbArgs > 2)
		throw std::runtime_error("Error: root can't have more than one argument");
	location.setRoot(args[1]);
}
void	setArgMethods(Location &location, std::vector<std::string> &args)
{
	size_t	nbArgs = 0;

	args.erase(args.begin());
	for (size_t i = 0; i < args.size(); i++)
	{
		std::cout << "args" << i << " = " << args[i] << std::endl;
		if (i != 0 && (args[i] != "GET" && args[i] != "POST" && args[i] != "DELETE"))
			throw std::runtime_error("Error: allow_methods can only have GET, POST or DELETE as arguments");
		nbArgs++;
	}
	if (nbArgs < 2)
		throw std::runtime_error("Error: allow_methods must have at least one argument");
	(void)location;
	(void)args;

	std::cout << "methods detetected" << std::endl;
}
void	setArgIndex(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "index detetected" << std::endl;
}
void	setArgAutoIndex(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "autoindex detetected" << std::endl;
}
void	setArgRet(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "return detetected" << std::endl;
}
void	setArgCgiPath(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "cgi_path detetected" << std::endl;
}
void	setArgCgiExt(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "cgi_ext detetected" << std::endl;
}
void	setArgUploadStore(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
	std::cout << "upload_store detetected" << std::endl;
}
