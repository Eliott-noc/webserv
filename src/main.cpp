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
#define C "\033[36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

void run_test(int num, std::string title, std::vector<std::string> chunks, ServerConfig &config, int expected_code) {
    std::cout << BOLD << "[TEST " << std::setw(2) << num << "] " << std::left << std::setw(50) << title << RESET;
    
    Request req(num);
    int parse_res = 0;
    size_t limit = 1000;

    for (size_t i = 0; i < chunks.size(); ++i) {
        parse_res = req.parse(chunks[i], limit);
        if (parse_res > 1 && parse_res != 200) break;
    }

    Response res;
    if (parse_res == 200) res.makeResponse(req, config);
    else if (parse_res > 200) res.buildErrorPage(parse_res, config);
    else { std::cout << Y << " [WAITING] " << RESET << std::endl; return; }

    while (!res.isFinished()) res.sendResponse(-1);

    std::string full = res.getRawResponse();
    int actual_code = (full.length() > 12) ? atoi(full.substr(9, 3).c_str()) : 0;

    if (actual_code == expected_code)
        std::cout << G << " [ OK ] " << RESET << "(Code " << actual_code << ")" << std::endl;
    else
        std::cout << R << " [ FAIL ] " << RESET << "Got " << actual_code << " but expected " << expected_code << std::endl;
}

int main() {
    ServerConfig config;
    config.setRoot("./arena");

    // Config Locations
    Location l1; l1.setPath("/"); l1.addMethod("GET"); l1.addMethod("DELETE"); l1.setIndex("index.html"); config.addLocation(l1);
    Location l2; l2.setPath("/protected"); l2.addMethod("GET"); l2.setIndex("index.html"); config.addLocation(l2);
    Location l3; l3.setPath("/uploads"); l3.addMethod("POST"); l3.addMethod("GET"); l3.setUploadStore("./arena/uploads"); config.addLocation(l3);
    Location l4; l4.setPath("/cgi-bin"); l4.addMethod("GET"); l4.addMethod("POST"); l4.setCgiExt(".py"); l4.setCgiPath("/usr/bin/python3"); config.addLocation(l4);

    // 1. Fragmentation perverse (couper au milieu des délimiteurs)
    std::vector<std::string> t1;
    t1.push_back("GET / HTTP/1.1\r"); 
    t1.push_back("\nHost: localhost\r\n\r");
    t1.push_back("\n");
    run_test(1, "Fragmentation sur délimiteurs \\r\\n", t1, config, 200);

    // 2. Insensibilité à la casse (host: vs Host:)
    std::vector<std::string> t2;
    t2.push_back("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
    run_test(2, "Headers Case Insensitivity", t2, config, 200);

    // 3. Path Traversal complexe (encodé et sournois)
    std::vector<std::string> t3;
    t3.push_back("GET /uploads/../protected/./secret/../../index.html HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(3, "Path Traversal (Labyrinthe)", t3, config, 200);

    // 4. Tentative d'intrusion (Sortie de Root)
    std::vector<std::string> t4;
    t4.push_back("GET /../../../../etc/passwd HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(4, "Sécurité : Sortie de Root", t4, config, 400);

    // 5. Chunked Encoding : Taille invalide (Hexa corrompu)
    std::vector<std::string> t5;
    t5.push_back("POST /uploads/fail.txt HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n");
    t5.push_back("G\r\nBoom\r\n0\r\n\r\n"); // G n'est pas de l'hexa
    run_test(5, "Chunked : Taille invalide (G\\r\\n)", t5, config, 400);

    // 6. Content-Length : Mensonge (Annonce 10, envoie 100)
    std::vector<std::string> t6;
    t6.push_back("POST /uploads/liar.txt HTTP/1.1\r\nHost: a\r\nContent-Length: 10\r\n\r\n");
    t6.push_back("CeciFaitPlusDeDixCaracteres");
    run_test(6, "Content-Length : Trop de données", t6, config, 200); // Doit s'arrêter à 10

    // 7. Méthode inconnue (Protocole)
    std::vector<std::string> t7;
    t7.push_back("MAGIC / HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(7, "Méthode non implémentée (MAGIC)", t7, config, 400);

    // 8. CGI : POST avec gros Body fragmenté
    std::vector<std::string> t8;
    t8.push_back("POST /cgi-bin/test.py HTTP/1.1\r\nHost: a\r\nContent-Length: 20\r\n\r\n");
    t8.push_back("Ligne 1\n");
    t8.push_back("Ligne 2\n");
    run_test(8, "CGI : POST Body fragmenté", t8, config, 200);

    // 9. Sécurité : Header trop long (> 8Ko) en un seul morceau
    std::vector<std::string> t9;
    t9.push_back("GET / HTTP/1.1\r\nLarge: " + std::string(9000, 'X') + "\r\n\r\n");
    run_test(9, "Attaque : Headers > 8Ko", t9, config, 431);

    // 10. DELETE sur un fichier inexistant dans une route autorisée
    std::vector<std::string> t10;
    t10.push_back("DELETE /ghost.txt HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(10, "DELETE : Fichier inexistant", t10, config, 404);

    std::vector<std::string> t11;
    t11.push_back("GET /space%20folder/mon%20fichier.txt HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(11, "URL Decoding (%20 et espaces)", t11, config, 200);

    // --- T12 : Fichier Vide ---
    std::vector<std::string> t12;
    t12.push_back("GET /empty.txt HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(12, "Fichier vide (0 octet)", t12, config, 200);

    // --- T13 : CGI Crash ---
    std::vector<std::string> t13;
    t13.push_back("GET /cgi-bin/error.py HTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(13, "CGI Script Error (Syntax Error)", t13, config, 500);

    // --- T14 : Host header vide ---
    std::vector<std::string> t14;
    t14.push_back("GET / HTTP/1.1\r\nHost: \r\n\r\n");
    run_test(14, "Host header present mais vide", t14, config, 400);

    // --- T15 : Request Line avec tabulations ---
    std::vector<std::string> t15;
    t15.push_back("GET\t/\tHTTP/1.1\r\nHost: a\r\n\r\n");
    run_test(15, "Request Line avec Tabs au lieu d'espaces", t15, config, 400);

    return 0;
}
