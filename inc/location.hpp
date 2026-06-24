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
		bool						_auto_index;     // on/off
		std::string					_return;        // Redirection (301)
		std::string					_cgi_path;      // chemin vers python/php
		std::string					_cgi_ext;       // .py ou .php
		std::string					_upload_store;  // dossier d'upload

	public:
		Location();
		Location(const Location &other);
		~Location();
		
		Location	&operator=(const Location &other);

		void						addMethod(std::string method);
		
		std::string					getPath() const;
		std::string					getRoot() const;
		std::vector<std::string>	getMethods() const;
		bool						getAutoIndex() const;
		std::string					getIndex() const;
		std::string					getReturn() const;
		std::string					getCGIPath() const;
		std::string					getCGIExt() const;
		std::string					getUploadStore() const;

		void						setPath(const std::string &path);
		void						setRoot(const std::string &root);
		void						setIndex(const std::string &index);
		void						setAutoIndex(const bool &autoIndex);
		void						setRet(const std::string &ret);
		void						setCgiPath(const std::string &cgi_path);
		void						setCgiExt(const std::string &cgi_ext);
		void						setUploadStore(const std::string &uploadStore);
};

#endif
