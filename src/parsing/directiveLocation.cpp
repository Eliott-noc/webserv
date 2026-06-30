#include "../../inc/directiveLocation.hpp"

DirectiveLocation::DirectiveLocation()
{
	_root = false;
	_methods = false;
	_index = false;
	_auto_index = false;
	_return = false;
	_cgi_path = false;
	_cgi_ext = false;
	_upload_store = false;
}

void DirectiveLocation::setRoot()
{
	_root = true;
}

void DirectiveLocation::setMethods()
{
	_methods = true;
}

void DirectiveLocation::setIndex()
{
	_index = true;
}

void DirectiveLocation::setAutoIndex()
{
	_auto_index = true;
}

void DirectiveLocation::setReturn()
{
	_return = true;
}

void DirectiveLocation::setCgiPath()
{
	_cgi_path = true;
}

void DirectiveLocation::setCgiExt()
{
	_cgi_ext = true;
}

void DirectiveLocation::setUploadStore()
{
	_upload_store = true;
}

bool	DirectiveLocation::getRoot() const
{
	if (_root == true)
		return true;
	return false;
}

bool	DirectiveLocation::getMethods() const
{
	if (_methods == true)
		return true;
	return false;
}

bool	DirectiveLocation::getIndex() const
{
	if (_index == true)
		return true;
	return false;
}

bool	DirectiveLocation::getAutoIndex() const
{
	if (_auto_index == true)
		return true;
	return false;
}

bool	DirectiveLocation::getReturn() const
{
	if (_return == true)
		return true;
	return false;
}

bool	DirectiveLocation::getCgiPath() const
{
	if (_cgi_path == true)
		return true;
	return false;
}

bool	DirectiveLocation::getCgiExt() const
{
	if (_cgi_ext == true)
		return true;
	return false;
}

bool	DirectiveLocation::getUploadStore() const
{
	if (_upload_store == true)
		return true;
	return false;
}