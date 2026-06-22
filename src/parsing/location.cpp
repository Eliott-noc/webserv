#include "../../inc/location.hpp"

Location::Location()
	: _autoIndex(false)
{
}

Location::Location(const Location &other)
{
	*this = other;
}

Location &Location::operator=(const Location &other)
{
	if (this != &other)
	{
		_path = other._path;
		_root = other._root;
		_methods = other._methods;
		_index = other._index;
		_autoIndex = other._autoIndex;
		_return = other._return;
		_cgi_path = other._cgi_path;
		_cgi_ext = other._cgi_ext;
		_upload_store = other._upload_store;
	}
	return *this;
}

Location::~Location() {}

void	Location::addMethod(std::string method)
{
	_methods.push_back(method);
}

std::string	Location::getPath() const
{
	return _path;
}

std::string	Location::getRoot() const
{
	return _root;
}

std::vector<std::string>	Location::getMethods() const
{
	return _methods;
}

bool	Location::getAutoIndex() const
{
	return _autoIndex;
}

std::string	Location::getIndex() const
{
	return _index;
}

std::string	Location::getReturn() const
{
	return _return;
}

std::string	Location::getCGIPath() const
{
	return _cgi_path;
}

std::string	Location::getCGIExt() const
{
	return _cgi_ext;
}

std::string	Location::getUploadStore() const
{
	return _upload_store;
}

void	Location::setPath(std::string path)
{
	_path = path;
}

void	Location::setRoot(std::string root)
{
	_root = root;
}

void	Location::setIndex(std::string index)
{
	_index = index;
}

void	Location::setAutoIndex(bool autoIndex)
{
	_autoIndex = autoIndex;
}
