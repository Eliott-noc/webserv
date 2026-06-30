#include "utilsError.hpp"

void	printPortErr(int errcode, int port){
	std::cerr << "Error: " << strerror(errcode) << " on port " << port << std::endl;
}