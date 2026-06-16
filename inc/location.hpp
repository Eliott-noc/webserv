#ifndef location_HPP
# define location_HPP

#include "webserv.hpp"

class Location
{
	private:
		std::string					_path;          // ex: /images
		std::string					_root;          // ex: ./www/images
		std::vector<std::string>	_methods;       // GET, POST, DELETE
		std::string					_index;         // index.html
		bool						_autoindex;     // on/off
		std::string					_return;        // Redirection (301)
		std::string					_cgi_path;      // chemin vers python/php
		std::string					_cgi_ext;       // .py ou .php
		std::string					_upload_store;  // dossier d'upload
	
	public:
		Location();
		Location(const Location &src);
		~Location();
		
		Location	&operator=(const Location &src);
		// Getters et Setters pour chaque variable
		// ...
};

#endif