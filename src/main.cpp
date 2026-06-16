#include "../inc/webserv.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (std::cout << "Usage : ./webserv <config file>" << std::endl, 1);
	std::string	file = argv[1];
	
	std::cout << file << std::endl;
	//parsing();
	//networkInfrastructure();
	//translator();
	return 0;
}
