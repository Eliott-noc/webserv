#include "../../inc/include.hpp"
#include "../../inc/locArgs.hpp"
#include "../../inc/directiveServer.hpp"

int	checkDuplicateIndex(const std::vector<std::string> &args)
{
	for (size_t i = 0; i < args.size(); i++)
	{
		for (size_t j = i + 1; j < args.size(); j++)
		{
			if (args[i] == args[j])
				return 1;
		}
	}
	return 0;
}

int	checkDuplicateMethods(const std::string &arg, t_methods *methods)
{
	if (arg == "GET")
	{
		if (methods->_get == 1)
			return 1;
		methods->_get = 1;
	}
	else if (arg == "POST")
	{
		if (methods->_post == 1)
			return 1;
		methods->_post = 1;
	}
	else if (arg == "DELETE")
	{
		if (methods->_delete == 1)
			return 1;
		methods->_delete = 1;
	}
	return 0;
}

int	isServKeyword(const std::string &str)
{
	if (str == "listen" || str == "server_name" || str == "root"
		|| str == "index" || str == "error_page" || str == "client_max_body_size"
		|| str == "host" || str == "location")
		return 1;

	return 0;
}

int	isLocKeyword(const std::string &str)
{
	if (str == "autoindex" || str == "allowed_methods" || str == "root"
		|| str == "index" || str == "return" || str == "upload_dir"
		|| str == "upload_store" || str == "alias" || str == "cgi"
		|| str == "cgi_extension" || str == "cgi_pass")
		return 1;

	return 0;
}
