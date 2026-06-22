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

int main()
{
	ServerConfig config;
	Location loc;

	loc.setPath("/");
	loc.setRoot("./www");
	loc.setIndex("index.html");
	loc.addMethod("GET");
	config.addLocation(loc);

	{
		std::cout << "\n=== TEST 1 : GET /index.html (200 OK) ===" << std::endl;
		Request req;
		std::string raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
		req.parse(raw, 1000);

		std::cout << "\nResponse:" << std::endl;
		Response res;
		res.makeResponse(req, config);
		std::cout << res.getRawResponse() << std::endl;
	}

	{
		std::cout << "\n=== TEST 2 : GET /introuvable.html (404 Not Found) ===" << std::endl;
		Request req;
		std::string raw = "GET /introuvable.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
		req.parse(raw, 1000);

		std::cout << "\nResponse:" << std::endl;
		Response res;
		res.makeResponse(req, config);
		std::cout << res.getRawResponse() << std::endl;
	}

	{
		std::cout << "\n=== TEST 3 : POST sur une route GET (405) ===" << std::endl;
		Request req;
		std::string raw = "POST /index.html HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n";
		req.parse(raw, 1000);

		std::cout << "\nResponse:" << std::endl;
		Response res;
		res.makeResponse(req, config);
		std::cout << res.getRawResponse() << std::endl;
	}

	return 0;
}

//result:
// === TEST 1 : GET /index.html (200 OK) ===
// code Text

// [Response] Status 200 generated.
// HTTP/1.1 200 OK
// Content-Length: [Taille de ton fichier]
// Content-Type: text/html
// Server: webserv/1.0

// [Ici le contenu de ton fichier www/index.html]

// === TEST 2 : GET /introuvable.html (404 Not Found) ===
// code Text

// [Response] Error 404 generated.
// HTTP/1.1 404 Not Found
// Content-Length: [Taille du HTML d'erreur]
// Content-Type: text/html
// Server: webserv/1.0

// <html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>webserv/1.0</center></body></html>

// === TEST 3 : POST sur une route GET (405) ===

// (Si tu as bien mis la condition pour refuser le POST ou si tu n'as pas encore codé le POST)
// code Text

// [Response] Error 405 generated.
// HTTP/1.1 405 Method Not Allowed
// Content-Length: [Taille du HTML d'erreur]
// Content-Type: text/html
// Server: webserv/1.0

// <html><head><title>405 Method Not Allowed</title></head><body><center><h1>405 Method Not Allowed</h1></center><hr><center>webserv/1.0</center></body></html>
