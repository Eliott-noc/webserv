// #include "serverManager.hpp"
// #include <iostream>
// #include <vector>

// // Make sure your compiler can see serverManager.cpp, serverConfig.cpp, 
// // and any associated source files (Request, Response, Client).
// int main() {
//     std::cout << "=== Starting ServerManager Isolation Test ===" << std::endl;

//     std::vector<ServerConfig> configs;

//     // 1. Set up a mock Server Configuration
//     ServerConfig config1;
    
//     // Configure maximum body size limits if needed
//     // config1.setClientMaxBodySize(1024 * 1024); // Example: 1 MB limit

//     // 2. Set up our listening ports/hosts
//     Listen interface1;
//     interface1._host = "127.0.0.1"; // Binds specifically to localhost
//     interface1._port = 8080;
    
//     Listen interface2;
//     interface2._host = "0.0.0.0";   // Binds to all interfaces
//     interface2._port = 8081;

//     // Adjust these lines to match how you append "Listen" structs to your ServerConfig class:
//     // e.g., if you have an addListen() helper, or a push_back on a reference:
//     config1.setListen(interface1);
//     config1.setListen(interface2);
    
//     // For this isolation test, we assume configs contains config1 with those ports set up
//     configs.push_back(config1);

//     // 3. (Optional) Set up a second virtual server configuration to test running multiple servers
//     /*
//     ServerConfig config2;
//     Listen interface3;
//     interface3._host = "127.0.0.1";
//     interface3._port = 9090;
//     config2.addListen(interface3);
//     configs.push_back(config2);
//     */

//     // 4. Instantiating and executing the manager loop
//     try {
//         ServerManager manager(configs);
//         manager.run();
//     } catch (const std::exception& e) {
//         std::cerr << "[FATAL] An unhandled exception escaped to main: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }