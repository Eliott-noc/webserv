#include "../inc/webserv.hpp"

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

int	main(int argc, char **argv)
{
	if (argc != 2)
		return (std::cout << "Error:\nUsage: ./webserv <config file>" << std::endl, 1);
	std::string	line = argv[1];

	Request	r;

	r.parse(line + "\r\n", 1000);

	return 0;
}
