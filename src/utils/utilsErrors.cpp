#include "utilsError.hpp"

void	printPortErr(int errcode, int port){
	std::cerr << "Fatal: " << strerror(errcode) << " on port " << port << std::endl;
}