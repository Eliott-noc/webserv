#include "utilsError.hpp"

void	printPortErr(int errcode, int port){
	if (port != -2)
		std::cerr << "Error: " << strerror(errcode) << " on port " << port << std::endl;
	else
		std::cerr << "Error: " << strerror(errcode) << std::endl;
}