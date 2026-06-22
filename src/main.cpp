#include "../inc/webserv.hpp"
#include "../inc/parser.hpp"

// int	main(int argc, char **argv)
// {
// 	if (argc != 2)
// 		return (std::cout << "Error:\nUsage: ./webserv <config file>" << std::endl, 1);
// 	std::string	file = argv[1];
	
// 	std::cout << file << std::endl;
// 	//parsing();
// 	//networkInfrastructure();
// 	//translator();
// 	return 0;
// }

void print_test_header(std::string title)
{
	std::cout << "\n========================================" << std::endl;
	std::cout << "TEST : " << title << std::endl;
	std::cout << "========================================" << std::endl;
}

int main()
{
	ServerConfig	config;
	Response		response;

	response.buildErrorPage(404, config);

	size_t limit = 100;

	{
		print_test_header("GET Simple");
		std::string req = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
		Request r;
		r.parse(req, limit);
	}

	{
		print_test_header("GET avec Query String");
		std::string req = "GET /search?query=chats&sort=desc&limit=10 HTTP/1.1\r\nHost: google.com\r\n\r\n";
		Request r;
		r.parse(req, limit);
	}

	{
		print_test_header("POST avec Body");
		std::string body = "nom=william&projet=webserv";
		std::stringstream ss;
		ss << "POST /form HTTP/1.1\r\n";
		ss << "Host: 127.0.0.1\r\n";
		ss << "Content-Length: " << body.length() << "\r\n";
		ss << "\r\n";
		ss << body;
		
		Request r;
		r.parse(ss.str(), limit);
	}

	{
		print_test_header("DELETE");
		std::string req = "DELETE /tmp/test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
		Request r;
		r.parse(req, limit);
	}

	{
		print_test_header("ERREUR 413 (Limite 100 octets)");
		std::string long_body(150, 'A');
		std::stringstream ss;
		ss << "POST /big_file HTTP/1.1\r\n";
		ss << "Content-Length: 150\r\n\r\n" << long_body;
		
		Request r;
		int res = r.parse(ss.str(), limit);
		std::cout << "CODE DE RETOUR : " << res << " (Attendu: 413)" << std::endl;
	}

	{
		print_test_header("ERREUR 400 (Body tronqué)");
		std::string req = "POST /fail HTTP/1.1\r\nContent-Length: 20\r\n\r\nHello";
		Request r;
		int res = r.parse(req, limit);
		std::cout << "CODE DE RETOUR : " << res << " (Attendu: 400)" << std::endl;
	}

	{
		print_test_header("Headers mal espacés");
		std::string req = "GET / HTTP/1.1\r\n"
						  "User-Agent:      Mozilla/5.0      \r\n"
						  "Custom-Header:value\r\n"
						  "\r\n";
		Request r;
		r.parse(req, limit);
	}

	{
		print_test_header("Body contenant des retours à la ligne");
		std::string body = "Ligne 1\r\nLigne 2\r\nLigne 3";
		std::stringstream ss;
		ss << "POST /text HTTP/1.1\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		
		Request r;
		r.parse(ss.str(), limit);
	}

	//parsing();
	//networkInfrastructure();
	//translator();
	return 0;
}
