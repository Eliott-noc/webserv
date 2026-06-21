#include "../../inc/location.hpp"

Location::Location()
	: _autoindex(false)
{
}

Location::Location(const Location &src)
{
	*this = src;
}

Location &Location::operator=(const Location &src)
{
	if (this != &src)
	{
		_path = src._path;
		_root = src._root;
		_methods = src._methods;
		_index = src._index;
		_autoindex = src._autoindex;
		_return = src._return;
		_cgi_path = src._cgi_path;
		_cgi_ext = src._cgi_ext;
		_upload_store = src._upload_store;
	}
	return *this;
}

Location::~Location()
{
}
