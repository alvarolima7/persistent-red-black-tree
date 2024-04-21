#include "RBTree.h"

#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> SplitOnSpace(std::string line)
{
	std::stringstream ss(line);
	std::string token;
	std::vector<std::string> tokens;
	while (ss >> token)
		tokens.push_back(token);

	return tokens;
}

void viewTree()
{
	RBTree tree;

	std::string line("");
	while (true)
	{
		std::getline(std::cin, line);
		if (line == "0")
			break;

		std::vector<std::string> tokens = SplitOnSpace(line);
		if (tokens.empty())
			continue;

		if (tokens.front() == "inc")
		{
			if (tokens.size() < 2)
			{
				std::cerr << "Error: INS command requires 2 arguments" << std::endl;
				continue;
			}

			int key = std::stoi(tokens[1]);
			tree.Insert(key);
			std::cout << "Inserted " << key << " on version " << tree.CurrentVersion() << std::endl;
		}
		else if (tokens.front() == "rem")
		{
			if (tokens.size() < 2)
			{
				std::cerr << "Error: REM command requires 2 argument" << std::endl;
				continue;
			}

			int key = std::stoi(tokens[1]);
			tree.Remove(key);
			std::cout << "Removed " << key << " on version " << tree.CurrentVersion() << std::endl;
		}
		else if (tokens.front() == "imp")
		{
			if (tokens.size() == 1)
			{
				std::cout << "\n\n Version: " << tree.CurrentVersion() << "\n\n";
				tree.Print();
			}
			else
			{
				int version = std::stoi(tokens[1]);
				std::cout << "\n\n Version: " << std::min(version, tree.CurrentVersion()) << "\n\n";
				tree.Print(version);
			}
		}
		else if (tokens.front() == "suc")
		{
			if (tokens.size() < 3)
			{
				std::cerr << "Error: SUC command requires 2 arguments" << std::endl;
				continue;
			}

			int key = std::stoi(tokens[1]);
			int version = std::stoi(tokens[2]);
			int successor = tree.Successor(key, version);
			std::cout << "\n\n Successor: " << (successor == INT32_MAX ? "Infinity" : std::to_string(successor)) << "\n\n";
		}
		else
			std::cerr << "Error: Unknown command " << tokens.front() << std::endl;
	}
}

int main()
{
	viewTree();
}