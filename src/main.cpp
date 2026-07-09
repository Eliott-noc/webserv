#include "../inc/webserv.hpp"
#include "../inc/parser.hpp"

// int	main(int argc, char **argv)
// {
// 	if (argc != 2)
// 		return (std::cout << "Error:\nUsage: ./webserv <config file>" << std::endl, 1);
// 	std::string	file = argv[1];

// 	std::vector<ServerConfig> servers = parseConfig(file);

// 	//parsing();
// 	//networkInfrastructure();
// 	//translator();
// 	return 0;
// }


#define R "\033[31m"
#define G "\033[32m"
#define Y "\033[33m"
#define B "\033[34m"
#define C "\033[36m"
#define RESET "\033[0m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

void run_ultimate_test(int num, std::string title, std::vector<std::string> chunks, ServerConfig &config, int expected_status, size_t limit = 5000)
{
	std::cout << BOLD << "[TEST " << std::setw(2) << num << "] " << std::left << std::setw(40) << title << RESET;
	
	Request req(num);
	int parse_res = 0;
	
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		parse_res = req.parse(chunks[i], limit);
		if (parse_res > 1) break;
	}

	int actual_status = 0;
	if (parse_res == 200)
	{
		Response res;
		res.makeResponse(req, config);
		std::string first_line = res.getRawResponse().substr(0, res.getRawResponse().find("\r\n"));
		std::stringstream ss(first_line.substr(9, 3));
		ss >> actual_status;
	} else
		actual_status = parse_res;

	if (actual_status == expected_status)
		std::cout << G << " [ OK ] " << RESET << "(Code " << actual_status << ")" << std::endl;
	else
		std::cout << R << " [ FAIL ] " << RESET << "Got " << actual_status << " but expected " << expected_status << std::endl;
}

int main() {
	ServerConfig config;
	config.setRoot("./test_death");

	Location l1; l1.setPath("/"); l1.addMethod("GET"); l1.addMethod("POST"); l1.addMethod("DELETE"); l1.setIndex("index.html"); config.addLocation(l1);
	Location l2; l2.setPath("/a/b"); l2.addMethod("GET"); l2.setIndex("index.html"); config.addLocation(l2);
	Location l3; l3.setPath("/uploads"); l3.addMethod("POST"); l3.addMethod("GET"); l3.setUploadStore("./test_death/uploads"); config.addLocation(l3);
	Location l4; l4.setPath("/empty"); l4.addMethod("GET"); l4.setAutoIndex(true); config.addLocation(l4);

	std::vector<std::string> t1;
	std::string raw1 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	for(size_t i=0; i<raw1.size(); ++i) t1.push_back(raw1.substr(i, 1));
	run_ultimate_test(1, "Fragmentation (1 char at a time)", t1, config, 200);

	std::vector<std::string> t2;
	t2.push_back("GET / HTTP/1.1\r\nHost: ");
	t2.push_back(std::string(9000, 'A'));
	t2.push_back("\r\n\r\n");
	run_ultimate_test(2, "Header Overflow (>8Ko)", t2, config, 431);

	std::vector<std::string> t3;
	t3.push_back("POST /uploads/chunk.txt HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n");
	t3.push_back("5\r\nHello\r\n0\r\n\r\n");
	run_ultimate_test(3, "Chunked Encoding Success", t3, config, 201);

	std::vector<std::string> t4;
	t4.push_back("GET /a/b/c/index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");
	run_ultimate_test(4, "Deep Routing (Longest Prefix)", t4, config, 200);

	std::vector<std::string> t5;
	t5.push_back("POST /uploads/fail.txt HTTP/1.1\r\nHost: localhost\r\n\r\n");
	run_ultimate_test(5, "Missing Length (Error 411)", t5, config, 411);

	std::vector<std::string> t6;
	t6.push_back("GET /secret.txt HTTP/1.1\r\nHost: localhost\r\n\r\n");
	run_ultimate_test(6, "Permissions (Error 403)", t6, config, 403);

	std::vector<std::string> t7;
	t7.push_back("GET /empty/ HTTP/1.1\r\nHost: localhost\r\n\r\n");
	run_ultimate_test(7, "Autoindex Empty Dir", t7, config, 200);

	std::vector<std::string> t8;
	t8.push_back("DELETE /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");
	run_ultimate_test(8, "DELETE existing file", t8, config, 204);

	std::vector<std::string> t9;
	t9.push_back("POST /uploads/fake.txt HTTP/1.1\r\nHost: a\r\nContent-Length: 100\r\n\r\nSmall");
	run_ultimate_test(9, "Body too small (Incomplete)", t9, config, 1);

	std::vector<std::string> t10;
	t10.push_back("GET / HTTP/1.1\r\n\r\n");
	run_ultimate_test(10, "Missing Host Header", t10, config, 400);

	return 0;
}
