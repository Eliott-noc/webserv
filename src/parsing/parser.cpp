#include "../../inc/parser.hpp"
#include "../../inc/utils.hpp"
#include "../../inc/serverConfig.hpp"
#include "../../inc/location.hpp"
#include "../../inc/locArgs.hpp"
#include "../../inc/serverArgs.hpp"

static void printLocation(const Location& location)
{
    std::cout << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "              LOCATION CONFIG           \n";
    std::cout << "----------------------------------------\n";

    // Path
    std::cout << "Path              : "
              << location.getPath()
              << '\n';

    // Root
    std::cout << "Root              : "
              << location.getRoot()
              << '\n';


    // Methods
    std::vector<std::string> methods = location.getMethods();

    std::cout << "Methods           : ";
    if (methods.empty())
        std::cout << "(none)";
    else
    {
        for (size_t i = 0; i < methods.size(); i++)
            std::cout << methods[i] << " ";
    }
    std::cout << '\n';


    // Index
    std::vector<std::string> index = location.getIndex();

    std::cout << "Index             : ";
    if (index.empty())
        std::cout << "(none)";
    else
    {
        for (size_t i = 0; i < index.size(); i++)
            std::cout << index[i] << " ";
    }
    std::cout << '\n';


    // Auto index
    std::cout << "Auto index        : "
              << (location.getAutoIndex() ? "on" : "off")
              << '\n';

    std::cout << "Auto index set    : "
              << (location.isAutoIndexSet() ? "yes" : "no")
              << '\n';

	std::cout << "Client max body size: "
			  << location.getClientMaxBodySize() << std::endl;

    // Return
    std::cout << "Return code       : "
              << location.getReturnCode()
              << '\n';

    std::cout << "Return URL        : "
              << location.getReturnUrl()
              << '\n';


    // CGI
    std::cout << "CGI path          : "
              << location.getCGIPath()
              << '\n';

    std::cout << "CGI extension     : "
              << location.getCGIExt()
              << '\n';


    // Upload
    std::cout << "Upload store      : "
              << location.getUploadStore()
              << '\n';


    // Error pages
    std::map<int, std::string> errors = location.getErrorPages();

    std::cout << "Error pages       :\n";

    if (errors.empty())
    {
        std::cout << "  (none)\n";
    }
    else
    {
        for (std::map<int, std::string>::iterator it = errors.begin();
             it != errors.end();
             ++it)
        {
            std::cout << "  "
                      << it->first
                      << " -> "
                      << it->second
                      << '\n';
        }
    }

    std::cout << "----------------------------------------\n";
}

static void printServer(const ServerConfig& server)
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "              SERVER CONFIG             \n";
    std::cout << "========================================\n";

    // Listen
    const std::vector<Listen>& listens = server.getListens();

    std::cout << "\n[LISTEN]\n";
    for (size_t i = 0; i < listens.size(); ++i)
    {
        std::cout << "  Listen #" << i << '\n';
        std::cout << "    Host : " << listens[i]._host << '\n';
        std::cout << "    Port : " << listens[i]._port << '\n';
    }

    // Root
    std::cout << "\n[ROOT]\n";
    std::cout << "  " << server.getRoot() << '\n';

    // Index
    const std::vector<std::string>& indexes = server.getIndex();

    std::cout << "\n[INDEX]\n";
    if (indexes.empty())
        std::cout << "  (none)\n";
    else
    {
        for (size_t i = 0; i < indexes.size(); ++i)
            std::cout << "  " << i << " : " << indexes[i] << '\n';
    }

	std::cout << "RETURN : " << server.getReturnCode() << ", " << server.getReturnUrl() << std::endl;
    // Autoindex
    std::cout << "\n[AUTOINDEX]\n";
    std::cout << "  " << (server.getAutoIndex() ? "on" : "off") << '\n';

    // Server names
    const std::vector<std::string>& names = server.getServerNames();

    std::cout << "\n[SERVER NAMES]\n";
    if (names.empty())
        std::cout << "  (none)\n";
    else
    {
        for (size_t i = 0; i < names.size(); ++i)
            std::cout << "  " << i << " : " << names[i] << '\n';
    }

    // Client body size
    std::cout << "\n[CLIENT MAX BODY SIZE]\n";
    std::cout << "  " << server.getClientMaxBodySize() << '\n';

    // Error pages
    const std::map<int, std::string>& errorPages = server.getErrorPages();

    std::cout << "\n[ERROR PAGES]\n";

    if (errorPages.empty())
        std::cout << "  (none)\n";
    else
    {
        for (std::map<int, std::string>::const_iterator it = errorPages.begin();
             it != errorPages.end();
             ++it)
        {
            std::cout << "  " << it->first
                      << " -> "
                      << it->second
                      << '\n';
        }
    }

    // Locations
    const std::vector<Location>& locations = server.getLocations();

    std::cout << "\n[LOCATIONS] (" << locations.size() << ")\n";

    for (size_t i = 0; i < locations.size(); ++i)
    {
        std::cout << "\n----------------------------------------\n";
        std::cout << "Location #" << i << '\n';
        std::cout << "----------------------------------------\n";

        printLocation(locations[i]);
    }

    std::cout << "========================================\n";
}

static std::string	readFile(const std::string &filename)
{
	std::string		line;
	std::string		content;
	std::ifstream	infile;

	infile.open(filename.c_str());
	while (std::getline(infile, line))
	{
		if (!content.empty())
			content += '\n';
		content += line;
	}
	return content;
}

static std::vector<std::string>	getTokens(std::string content)
{
	std::vector<std::string> tokens;
	std::string token;

	for (size_t i = 0; i < content.size(); i++)
	{
		char c = content[i];

		if (c == ' ' || c == '\t' || c == '\n')
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
			tokens.push_back(std::string(1, c));
		}
		else
		{
			token += c;
		}
	}
	if (!token.empty())
		tokens.push_back(token);

	return tokens;
}

static std::vector<std::vector<std::string> > extractServerBlocks(const std::vector<std::string> &tokens)
{
	int										depth = 0;
	bool									in_server = false;
	std::vector<std::string>				current;
	std::vector<std::vector<std::string> >	blocks;

	for(size_t i = 0; i < tokens.size(); i++)
	{
		if (!in_server)
		{
			if (tokens[i] == "server")
			{
				if (i + 1 >= tokens.size())
					throw std::runtime_error("Error: server without following token");
				if (tokens[i + 1] != "{")
					throw std::runtime_error("Error: server must be followed by '{'");
				in_server = true;
				current.clear();
				depth = 1;
				i++;
			}
			else
				throw std::runtime_error("Error: unexpected token outside server block");
			continue ;
		}
		current.push_back(tokens[i]);

		if (tokens[i] == "{")
			depth++;
		if (tokens[i] == "}")
		{
			depth--;
			if (depth < 0)
				throw std::runtime_error("Error: unexpected '}'");
		}
		if (in_server && depth == 0)
		{
			blocks.push_back(current);
			current.clear();
			in_server = false;
		}
	}
	if (in_server)
		throw std::runtime_error("Error: unclosed server block");

	return blocks;
}

static std::vector<std::string> extractLocationBlock(const std::vector<std::string> &block, size_t *i)
{
	std::vector<std::string>	extracted_block;

	while (*i < block.size() && block[*i] != "}")
	{
		if (block[*i] == "location" || block[*i] == "{")
		{
			(*i)++;
			continue ;
		}
		extracted_block.push_back(block[*i]);
		(*i)++;
	}
	if (*i == block.size())
		throw std::runtime_error("Error: missing '}' in location block");

	return extracted_block;
}

static void	setLocArgs(Location &location, std::vector<std::string> &args)
{
	if (args[0] == "root")
		setArgRoot(location, args);
	else if (args[0] == "allow_methods")
		setArgMethods(location, args);
	else if (args[0] == "index")
		setArgIndex(location, args);
	else if (args[0] == "autoindex")
		setArgAutoIndex(location, args);
	else if (args[0] == "return")
		setArgRet(location, args);
	else if (args[0] == "cgi_path")
		setArgCgiPath(location, args);
	else if (args[0] == "cgi_ext")
		setArgCgiExt(location, args);
	else if (args[0] == "upload_store")
		setArgUploadStore(location, args);
	else if (args[0] == "error_page")
		setArgErrorPage(location, args);
}

static Location	parseLocation(const std::vector<std::string> &l_block, const ServerConfig &server)
{
	Location				location;
	std::vector<std::string> args;
	(void)server;

	setArgPath(location, l_block[0]);

	for (size_t i = 1; i < l_block.size(); i++)
	{
		if ((i == 1 && l_block[i] == ";") || (l_block[i] == ";" && l_block[i - 1] == ";") || (i == l_block.size() - 1 && l_block[i] != ";"))
			throw std::runtime_error("Error: Empty directive");
		if (isLocKeyword(l_block[i]) && (l_block[i - 1] != ";" && i != 1))
			throw std::runtime_error("Error: keyword not in start of directive");
		if (l_block[i - 1] == ";" && !isLocKeyword(l_block[i]))
			throw std::runtime_error("Error: no keyword at start of location directive");
		if (l_block[i] != ";")
			args.push_back(l_block[i]);
		else
		{
			setLocArgs(location, args);
			args.clear();
		}
	}
	//checkLocation  si pas de index rempli, alors la location herite de l'index du server
	//De william: j'ai rajoute server comme parametre de la fonction pour pouvoir get Index du server
	//checkLocation(location, server);

	return location;
}

static void	parseServerDirective(ServerConfig &server, std::vector<std::string> &args)
{
	if (args[0] == "listen")
		setServerListen(server, args);
	else if (args[0] == "host")
		setServerHost(server, args);
	else if (args[0] == "server_name")
		setServerName(server, args);
	else if (args[0] == "root")
		setServerRoot(server, args);
	else if (args[0] == "index")
		setServerIndex(server, args);
	else if (args[0] == "error_page")
		setServerErrorPage(server, args);
	else if (args[0] == "client_max_body_size")
		setServerBodySize(server, args);
	else if (args[0] == "return")
		setServerRet(server, args);
}

static std::vector<std::string> extractServerDirective(const std::vector<std::string> &block, size_t *i)
{
	std::vector<std::string>	extracted_block;

	while (*i < block.size())
	{
		if (block[*i] == ";")
			break ;
		extracted_block.push_back(block[*i]);
		(*i)++;
	}
	return extracted_block;
}

static ServerConfig	parseServer(std::vector<std::string> &s_block)
{
	ServerConfig			server;
	std::vector<Location>	locations;

	for (size_t i = 0; i < s_block.size(); i++)
	{
		if (s_block[i] == "location")
		{
			std::vector<std::string> location_block = extractLocationBlock(s_block, &i);
			locations.push_back(parseLocation(location_block, server));
		}
		else
		{
			std::vector<std::string> serverDirective = extractServerDirective(s_block, &i);
			parseServerDirective(server, serverDirective);
		}
	}
	server.setLocations(locations);
	return server;
}

std::vector<ServerConfig>	parseConfig(const std::string &filename)
{
	std::vector<ServerConfig>	servers;
	std::string					content;
	std::vector<std::string>	tokens;
	std::vector<std::vector<std::string> > server_blocks;

	content = readFile(filename);
	tokens = getTokens(content);
	server_blocks = extractServerBlocks(tokens);

	for (size_t i = 0; i < server_blocks.size(); i++)
	{
		ServerConfig server = parseServer(server_blocks[i]);
		std::vector<Location> locations = server.getLocations();
		for (size_t j = 0; j < locations.size(); j++)
			checkLocation(&locations[j], server);
		server.setLocations(locations);
		servers.push_back(server);
	}

	//checkServers(servers);

	for (size_t i = 0; i < servers.size(); i++)
	{
		std::cout << "====SERVER NUMERO " << i << "====" << std::endl;
		printServer(servers[i]);
	}

	return servers;
}
