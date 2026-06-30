#include "../../inc/locArgs.hpp"
#include "../../inc/include.hpp"
#include "../../inc/utils.hpp"

void	setArgPath(Location &location, const std::string &path)
{
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
	t_methods	methods;
	methods._get = 0;
	methods._post = 0;
	methods._delete = 0;
	size_t		nbArgs = 0;

	std::cout << "methods detected" << std::endl;
	args.erase(args.begin());
	for (size_t i = 0; i < args.size(); i++)
	{
		if (args[i] != "GET" && args[i] != "POST" && args[i] != "DELETE")
			throw std::runtime_error("Error: allow_methods can only have GET, POST or DELETE as arguments");
		if (checkDuplicateMethods(args[i], &methods))
			throw std::runtime_error("Error: duplicate methods in location");
		nbArgs++;
	}
	if (nbArgs < 1)
		throw std::runtime_error("Error: allow_methods must have at least one argument");
	location.setMethods(args);
}
void	setArgIndex(Location &location, std::vector<std::string> &args)
{
	(void)location;
	args.erase(args.begin());
	if (args.empty())
		throw std::runtime_error("Error: index must have at least one argument");
	if (checkDuplicateIndex(args))
		throw std::runtime_error("Error: duplicate index in location");
	for (size_t i = 0; i < args.size(); i++)
	{
	}
}
void	setArgAutoIndex(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
}
void	setArgRet(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
}
void	setArgCgiPath(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
}
void	setArgCgiExt(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
}
void	setArgUploadStore(Location &location, const std::vector<std::string> &args)
{
	(void)location;
	(void)args;
}
