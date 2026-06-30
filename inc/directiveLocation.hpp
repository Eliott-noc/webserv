#ifndef DIRECTIVELOCATION_HPP
#define DIRECTIVELOCATION_HPP

class DirectiveLocation
{
	private:
		bool	_root;
		bool	_methods;
		bool	_index;
		bool	_auto_index;
		bool	_return;
		bool	_cgi_path;
		bool	_cgi_ext;
		bool	_upload_store;

	public:
		DirectiveLocation();

		void	setRoot();
		void	setMethods();
		void	setIndex();
		void	setAutoIndex();
		void	setReturn();
		void	setCgiPath();
		void	setCgiExt();
		void	setUploadStore();

		bool	getRoot() const;
		bool	getMethods() const;
		bool	getIndex() const;
		bool	getAutoIndex() const;
		bool	getReturn() const;
		bool	getCgiPath() const;
		bool	getCgiExt() const;
		bool	getUploadStore() const;
};

#endif