#include "../../inc/location.hpp"

Location::Location()
	: _auto_index(false)
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
		_auto_index = other._auto_index;
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
	return _auto_index;
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

void	Location::setPath(const std::string &path)
{
	_path = path;
}

void	Location::setRoot(const std::string &root)
{
	_root = root;
}

void	Location::setIndex(const std::string &index)
{
	_index = index;
}

void	Location::setAutoIndex(const bool &auto_index)
{
	_auto_index = auto_index;
}

void	Location::setRet(const std::string &ret)
{
	_return = ret;
}

void	Location::setCgiPath(const std::string &cgi_path)
{
	_cgi_path = cgi_path;
}

void	Location::setCgiExt(const std::string &cgi_ext)
{
	_cgi_ext = cgi_ext;
}

void	Location::setUploadStore(const std::string &uploadStore)
{
	_upload_store = uploadStore;
}
