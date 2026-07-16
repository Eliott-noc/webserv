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
		std::vector<std::string>	_index;         // index.html
		bool						_auto_index;     // on/off
		bool						_isAutoIndexSet;
		size_t						_client_max_body_size;
		int							_return_code;
		std::string					_return_url;
		std::string					_cgi_path;      // chemin vers python/php
		std::string					_cgi_ext;       // .py ou .php
		std::string					_upload_store;  // dossier d'upload
		std::map<int, std::string>	_error_pages;

	public:
		Location();
		Location(const Location &other);
		~Location();

		Location	&operator=(const Location &other);

		void						addMethod(std::string method);

		std::string					getPath() const;
		std::string					getRoot() const;
		std::vector<std::string>	getMethods() const;
		std::vector<std::string>	getIndex() const;
		bool						getAutoIndex() const;
		size_t						getClientMaxBodySize() const;
		int							getReturnCode() const;
		std::string					getReturnUrl() const;
		std::string					getCGIPath() const;
		std::string					getCGIExt() const;
		std::string					getUploadStore() const;
		std::map<int, std::string>	getErrorPages() const;
		

		void						setPath(const std::string &path);
		void						setRoot(const std::string &root);
		void						setMethods(const std::vector<std::string> &methods);
		void						setIndex(const std::string &index);
		void						setIndex(const std::vector<std::string> &index);
		void						setAutoIndex(const bool &autoIndex);
		bool						isAutoIndexSet() const;
		void						setClientMaxBodySize(const size_t &client_max_body_size);
		void						setRet(int &ret_code, const std::string &ret_url);
		void						setCgiPath(const std::string &cgi_path);
		void						setCgiExt(const std::string &cgi_ext);
		void						setUploadStore(const std::string &uploadStore);
		void						setErrorPages(const std::map<int, std::string> &error_pages);
};

#endif
