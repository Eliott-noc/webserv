#include "../../inc/parser.hpp"
#include "../../inc/utils.hpp"
#include "../../inc/location.hpp"
#include "../../inc/locArgs.hpp"
#include "../../inc/directiveServer.hpp"
#include "../../inc/directiveLocation.hpp"

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

static std::vector<std::string> extractLocationBlock(const std::vector<std::string> &block, size_t i)
{
	std::vector<std::string>	extracted_block;

	while (i < block.size() && block[i] != "}")
	{
		if (block[i] == "location" || block[i] == "{")
		{
			i++;
			continue ;
		}
		extracted_block.push_back(block[i]);
		i++;
	}
	if (i == block.size())
		throw std::runtime_error("Error: missing '}' in location block");

	return extracted_block;
}

static int	setLocArgs(Location &location, DirectiveLocation directiveLocation, std::vector<std::string> &args)
{
	std::cout << "test args[0] = " << args[0] << std::endl;
		if (args[0] == "root")
		{
			if (directiveLocation.getRoot() == true)
				return 1;
			directiveLocation.setRoot();
			setArgRoot(location, args);
		}
		else if (args[0] == "allow_methods")
		{
			std::cout << "pouet" << std::endl;
			if (directiveLocation.getMethods() == true)
			{
				std::cout << "return 1" << std::endl;
				return 1;
			}
			directiveLocation.setMethods();
			setArgMethods(location, args);
		}
		else if (args[0] == "index")
		{
			if (directiveLocation.getIndex() == true)
				return 1;
			directiveLocation.setIndex();
			setArgIndex(location, args);
		}
		else if (args[0] == "autoindex")
		{
			if (directiveLocation.getAutoIndex() == true)
				return 1;
			directiveLocation.setAutoIndex();
			setArgAutoIndex(location, args);
		}
		else if (args[0] == "return")
		{
			if (directiveLocation.getReturn() == true)
				return 1;
			directiveLocation.setReturn();
			setArgRet(location, args);
		}
		else if (args[0] == "cgi_path")
		{
			if (directiveLocation.getCgiPath() == true)
				return 1;
			directiveLocation.setCgiPath();
			setArgCgiPath(location, args);
		}
		else if (args[0] == "cgi_ext")
		{
			if (directiveLocation.getCgiExt() == true)
				return 1;
			directiveLocation.setCgiExt();
			setArgCgiExt(location, args);
		}
		else if (args[0] == "upload_store")
		{
			if (directiveLocation.getUploadStore() == true)
				return 1;
			directiveLocation.setUploadStore();
			setArgUploadStore(location, args);
		}
		std::cout << "arg = " << args[0] << std::endl;
		std::cout << "methods = " << directiveLocation.getMethods() << std::endl;
		return 0;
}

static Location	parseLocation(const std::vector<std::string> &l_block)
{
	Location				location;
	DirectiveLocation		directiveLocation;
	std::vector<std::string> args;

	setArgPath(location, l_block[0]);

	for (size_t i = 1; i < l_block.size(); i++)
	{
		//std::cout << i << " = " << l_block[i] << std::endl;
		if ((i == 1 && l_block[i] == ";") || (l_block[i] == ";" && l_block[i - 1] == ";") || (i == l_block.size() - 1 && l_block[i] != ";"))
			throw std::runtime_error("Error: Empty directive");
		if (isLocKeyword(l_block[i]) && (l_block[i - 1] != ";" && i != 1))
			throw std::runtime_error("Error: keyword not in start of directive");

		if (l_block[i] != ";")
			args.push_back(l_block[i]);
		else
		{
			if (setLocArgs(location, directiveLocation, args))
				throw std::runtime_error("Error: duplicate directive in location");
			args.clear();
		}
	}
	//checkLocation  si pas de index rempli, alors la location herite de l'index du server

	return location;
}


static ServerConfig	parseServer(const std::vector<std::string> &s_block)
{
	DirectiveServer			directiveServer;
	ServerConfig			server;
	std::vector<Location>	locations;

	for (size_t i = 0; i < s_block.size(); i++)
	{
		if (s_block[i] == "location")
		{
			std::vector<std::string> location_block = extractLocationBlock(s_block, i);
			// for (size_t j = 0; j < location_block.size(); j++)
			// 	std::cout << "location " << j << " = " << location_block[j] << std::endl;
			locations.push_back(parseLocation(location_block));
			//i = skipBlock(s_block, i);
		}
		// else
		// 	parseDirective(server, s_block, i);
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
		// servers.push_back(server);
	}

	//checkServers(servers);

	// for(size_t i = 0; i < server_blocks.size(); i++)
	// {
	// 	for(size_t j = 0; j < server_blocks[i].size(); j++)
	// 	{
	// 		std::cout << "block " << i << " token " << j << " = " << server_blocks[i][j] << std::endl;
	// 	}
	// }
	return servers;
}
