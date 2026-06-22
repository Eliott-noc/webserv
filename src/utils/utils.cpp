#include "../../inc/include.hpp"

int	isKeyword(const std::string &str)
{
	if (str == "listen" || str == "server_name" || str == "root"
		|| str == "index" || str == "error_page" || str == "client_max_body_size"
		|| str == "host")
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