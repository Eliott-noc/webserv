#include "../../inc/locArgs.hpp"
#include "../../inc/include.hpp"
#include "../../inc/utils.hpp"

void	setArgPath(Location &location, const std::string &path)
{
	if (path.empty())
		throw std::runtime_error("Error: location needs a path");
	if (path[0] != '/')
		throw std::runtime_error("Error : location path must be an absolute path");
	location.setPath(path);
}
void	setArgRoot(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: root must have exactly one argument");
	location.setRoot(args[1]);
}
void	setArgMethods(Location &location, const std::vector<std::string> &args)
{
	t_methods	methods;
	methods._get = 0;
	methods._post = 0;
	methods._delete = 0;

	for (size_t i = 1; i < args.size(); i++)
	{
		if (args[i] != "GET" && args[i] != "POST" && args[i] != "DELETE")
			throw std::runtime_error("Error: allow_methods can only have GET, POST or DELETE as arguments");
		if (checkDuplicateMethods(args[i], &methods))
			throw std::runtime_error("Error: duplicate methods in location");
	}
	location.setMethods(args);
}
void	setArgIndex(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: location index must have one argument");
	location.setIndex(args[1]);
}
void	setArgAutoIndex(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: autoindex must have one argument");
	if (args[1] == "on")
		location.setAutoIndex(true);
	else if (args[1] == "off")
		location.setAutoIndex(false);
	else
		throw std::runtime_error("Error: autoindex can only have 'on' or 'off' as arguments");
}
void	setArgRet(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2 && args.size() != 3)
		throw std::runtime_error("Error: return must have one or two argument");

	int	code;
	std::stringstream ss(args[1]);
	ss >> code;

	if (ss.fail() || !ss.eof())
		throw std::runtime_error("Error: invalid number");
	if (code < 100 || code > 599)
		throw std::runtime_error("Error: invalid http code");
	if (args.size() == 1)
		location.setRet(code , "");
	else
		location.setRet(code, args[2]);
}
void	setArgCgiPath(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
	throw std::runtime_error("Error: cgi_path must have one argument");

	const std::string &path = args[1];

	if (path.empty() || path[0] != '/')
		throw std::runtime_error("Error: cgi_path must be an absolute path");
	location.setCgiPath(path);
}
void	setArgCgiExt(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: cgi_ext must have one argument");
	const std::string &ext = args[1];
	if (ext.empty() || ext[0] != '.')
		throw std::runtime_error("Error: cgi_ext must start with '.'");
	location.setCgiExt(ext);
}
void	setArgUploadStore(Location &location, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		throw std::runtime_error("Error: upload_store must have one argument");

	const std::string &upload = args[1];

	if (upload.empty() || upload[0] != '/')
		throw std::runtime_error("Error: upload_store must be an absolute path");
	location.setUploadStore(upload);
}

// void	setArgErrorPage(Location &location, const std::vector<std::string> &args)
// {
// 	if (args.size() != 2 && args.size() != 3)
// 		throw std::runtime_error("Error: error_page must have one or two argument");

// 	int	code;
// 	std::stringstream ss(args[1]);
// 	ss >> code;

// 	if (ss.fail() || !ss.eof())
// 		throw std::runtime_error("Error: invalid number");
// 	if (code < 100 || code > 599)
// 		throw std::runtime_error("Error: invalid http code");
// 	if (args.size() == 1)
// 		location.setErrorPage(code , "");
// 	else
// 		location.setErrorPage(code, args[2]);
// }
