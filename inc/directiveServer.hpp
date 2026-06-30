#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

class DirectiveServer
{
		bool	_ports;
		bool	_host;
		bool	_root;
		bool	_index;
		bool	_auto_index;
		bool	_server_names;
		bool	_client_max_body_size;

	public:
		DirectiveServer();

		void	setPorts();
		void	setHost();
		void	setRoot();
		void	setIndex();
		void	setAutoIndex();
		void	setServerNames();
		void	setClientMaxBodySize();
};

#endif