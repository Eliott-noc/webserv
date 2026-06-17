#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "include.hpp"

/*
Permet de stocker les règles spécifiques à une route (URL) pour gérer les cas où
certains dossiers demandent un traitement particulier, comme autoriser uniquement
le POST dans un dossier /upload, activer l'affichage des fichiers (autoindex) dans
/images, ou rediriger une ancienne URL vers une nouvelle.
*/

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
		Location(const Location &other);
		~Location();
		
		Location	&operator=(const Location &other);
		// Getters et Setters pour chaque variable
		// ...
};

#endif