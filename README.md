*This project has been created as part of the 42 curriculum by jroh, whollebe and ele-moig.*

# Webserv

## Description
Webserv is a fully functional HTTP/1.0-compliant web server written in C++ 98. The primary goal of this project is to understand the inner workings of network protocols and multiplexed I/O operations. 

The server is designed to be entirely non-blocking, using a single socket multiplexer—such as `poll()`, `select()`, or `epoll()`—to monitor both read and write readiness across multiple listening ports and client connections simultaneously.
In our case, we used only `poll()`: every socket (listening sockets, client connections, and CGI pipe file descriptors) is registered in a single `poll()` loop, and no `read()`/`recv()` or `write()`/`send()` on a waitable descriptor happens without going through it first.

## Features
- **Static Website Hosting**: Serves HTML, CSS, JavaScript, and media files, with optional directory listing and configurable default index files.
- **HTTP Methods**: Full support for `GET`, `POST`, and `DELETE` requests.
- **File Uploads**: Allows clients to upload files to designated storage directories.
- **CGI Execution**: Supports execution of CGI scripts (e.g. Python, PHP) based on file extension, including chunked request un-chunking and EOF-based CGI output handling.
- **Multiple Servers / Ports**: Listens on several interface:port pairs at once, each serving different content.
- **Custom Configuration**: Parses an NGINX-inspired configuration file to define server limits, listening ports, routes, default error pages, redirections, and directory listing options.
- **Default Error Pages**: Built-in error pages are served automatically when none are configured.
- **Non-Blocking I/O**: A single `poll()` call drives all client and CGI I/O; the server never crashes or hangs indefinitely on a request.

## Configuration File
The server is configured via an NGINX-inspired `.conf` file passed as the program's argument. A minimal example:

```nginx
server {
    listen       8080;
    host         127.0.0.1;
    server_name  example.local;
    client_max_body_size 10M;

    error_page 404 /errors/404.html;

    location / {
        root            www/;
        index           index.html;
        methods         GET POST DELETE;
        autoindex       off;
    }

    location /upload {
        root            www/uploads;
        methods         POST DELETE;
        autoindex       on;
    }

    location /cgi-bin {
        root            www/cgi-bin;
        cgi_extension   .py;
        methods         GET POST;
    }
}
```

Each `server` block defines a listening interface:port pair; each `location` block scopes rules (allowed methods, root directory, redirection, autoindex, uploads, CGI) to a URL prefix. Sample configuration files and a matching test website are provided in the repository so every feature can be exercised during evaluation.

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

## Testing
The server can be tested with any standard web browser, `curl`, or raw `telnet` sessions for low-level protocol checks (headers, chunked encoding, connection handling). Behaviour was cross-checked against NGINX for status codes and header differences between HTTP versions. Sample configuration files and a demo website (including CGI scripts and an upload route) are included in the repository so each feature can be verified independently.

## Resources

### References
- **Beej's Guide to Network Programming**: [https://beej.us/guide/bgnet/](https://beej.us/guide/bgnet/)
- **RFC 1945 (HTTP/1.0)**: [https://datatracker.ietf.org/doc/html/rfc1945](https://datatracker.ietf.org/doc/html/rfc1945)
- **C++ Reference**: [https://en.cppreference.com/cpp](https://en.cppreference.com/cpp)
- **Linux Man Pages**: [https://man7.org/linux/man-pages/index.html](https://man7.org/linux/man-pages/index.html)

- **NGINX Docs** : [https://nginx.org/en/docs/beginners_guide.html](https://nginx.org/en/docs/beginners_guide.html)

### AI Usage Statement
AI tools were utilized during the development of this project to assist with research and design tasks. Specifically:
- **Architecture and Brainstorming**: AI was used to discuss structural ideas for the non-blocking state machine and understand the lifecycle of client sockets.
- **Debugging & Concepts**: AI was consulted to clarify edge cases involving HTTP chunked transfer encoding and how the CGI child processes communicate EOF to the parent via pipes.
All AI-suggested concepts and draft structures were manually reviewed, adapted to strictly comply with the C++98 standard, and verified through peer discussion to ensure full comprehension and technical integrity.
- **Writing README**: almost all of this readme was written by AI, because who writes these things anymore, but hey i'm still here :)