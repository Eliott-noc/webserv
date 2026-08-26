#include "../inc/webserv.hpp"

int	main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	if (argc != 2)
		return (std::cout << "Error:\nUsage: ./webserv <config file>" << std::endl, 1);
	std::string	file = argv[1];

	try {
		std::vector<ServerConfig> servers = parseConfig(file);
		ServerManager manager(servers);
		manager.run();

	} catch (const std::exception& e) {
		std::cerr << "[FATAL] An unhandled exception occurred: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}