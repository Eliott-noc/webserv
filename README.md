*This project has been created as part of the 42 curriculum by jroh, whollebe and ele-moig.*

# Webserv

## Description
Webserv is a fully functional HTTP/1.0-compliant web server written in C++ 98. The primary goal of this project is to understand the inner workings of network protocols and multiplexed I/O operations. 

The server is designed to be entirely non-blocking, using a single socket multiplexer—such as `poll()`, `select()`, or `epoll()`—to monitor both read and write readiness across multiple listening ports and client connections simultaneously.
In our case, we used only poll().

Key features of this implementation include:
- **Static Website Hosting**: Serves HTML, CSS, JavaScript, and media files.
- **HTTP Methods**: Full support for `GET`, `POST`, and `DELETE` requests.
- **File Uploads**: Allows clients to upload files to designated storage directories.
- **CGI Execution**: Supports the execution of common gateway interface (CGI) scripts (such as Python or PHP) based on file extensions.
- **Custom Configuration**: Parses an NGINX-inspired configuration file to define server limits, listening ports, routes, default error pages, redirection, and directory listing options.

## Instructions

### Compilation
The project requires a compiler compatible with C++98 standards. You can compile the executable using the provided `Makefile`:

```bash
make
```

This will compile the source files with the flags `-Wall -Wextra -Werror -std=c++98` and produce the `webserv` executable.

### Execution
To start the web server, run the executable with a configuration file as an argument:

```bash
./webserv path/to/config.conf
```

If no configuration file is provided, the program can be set to load a default configuration path.

### Cleaning Up
To remove object files:
```bash
make clean
```

To remove object files and the executable:
```bash
make fclean
```

To recompile from scratch:
```bash
make re
```

## Resources

### References
- **Beej's Guide to Network Programming**: [https://beej.us/guide/bgnet/](https://beej.us/guide/bgnet/)
- **RFC 1945 (HTTP/1.0)**: [https://datatracker.ietf.org/doc/html/rfc1945](https://datatracker.ietf.org/doc/html/rfc1945)
- **C++ Reference**: [https://en.cppreference.com/cpp](https://en.cppreference.com/cpp)
- **Linux Man Pages**: [https://man7.org/linux/man-pages/index.html](https://man7.org/linux/man-pages/index.html)

### AI Usage Statement
AI tools were utilized during the development of this project to assist with research and design tasks. Specifically:
- **Architecture and Brainstorming**: AI was used to discuss structural ideas for the non-blocking state machine and understand the lifecycle of client sockets.
- **Debugging & Concepts**: AI was consulted to clarify edge cases involving HTTP chunked transfer encoding and how the CGI child processes communicate EOF to the parent via pipes.
All AI-suggested concepts and draft structures were manually reviewed, adapted to strictly comply with the C++98 standard, and verified through peer discussion to ensure full comprehension and technical integrity.
- **Writing README**: almost all of this readme was written by AI, because who writes this things anymore, but hey i'm still here :)
