#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"
# include "request.hpp"
# include "response.hpp"

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
		Client(const Client &src);
		~Client();
		
		Client &operator=(const Client &src);
};

#endif