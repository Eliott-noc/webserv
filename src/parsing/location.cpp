#include "../../inc/location.hpp"
#include "../../inc/utils.hpp"

Location::Location()
	: _auto_index(false), _isAutoIndexSet(false),_return_code(0)
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
		_return_code = other._return_code;
		_return_url = other._return_url;
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

std::vector<std::string> Location::getIndex() const
{
	return _index;
}

int	Location::getReturnCode() const
{
	return _return_code;
}

std::string Location::getReturnUrl() const
{
	return _return_url;
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

std::map<int, std::string>	Location::getErrorPages() const
{
	return _error_pages;
}


void	Location::setPath(const std::string &path)
{
	_path = path;
}

void	Location::setRoot(const std::string &root)
{
	_root = root;
}

void	Location::setMethods(const std::vector<std::string> &methods)
{
	_methods = methods;
}

void	Location::setIndex(const std::string &index)
{
	_index.push_back(index);
	if (checkDuplicateIndex(_index))
		throw std::runtime_error("Error: duplicate index in location");
}

void	Location::setAutoIndex(const bool &auto_index)
{
	_auto_index = auto_index;
	_isAutoIndexSet = true;
}

bool Location::isAutoIndexSet() const
{
	return _isAutoIndexSet;
}

void	Location::setRet(int &ret_code, const std::string &ret_url)
{
	_return_code = ret_code;
	if (ret_url != "0")
		_return_url = ret_url;
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
