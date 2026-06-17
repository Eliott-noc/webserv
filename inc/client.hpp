#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "include.hpp"
# include "request.hpp"
# include "response.hpp"

/*
Permet de conserver l'état et les données de chaque visiteur pour gérer les cas de
transferts lents ou fragmentés, en stockant le texte reçu petit à petit dans un
buffer jusqu'à ce que la requête soit complète et prête à être traduite.
*/

class Client
{
	public:
		int			fd;
		std::string	raw_request_buffer; // Ce que le moteur lit du socket
		Request		request;
		Response	response;
		bool		request_is_complete;
		bool		response_is_ready;

		Client(int client_fd);
		Client(const Client &other);
		~Client();
		
		Client &operator=(const Client &src);
};

#endif