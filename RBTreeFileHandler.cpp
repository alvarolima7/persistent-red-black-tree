#include "RBTree.h"

#include <fstream>
#include <sstream>
#include <string>

class RBTreeFileHandler
{
public:
	inline RBTreeFileHandler(std::string inputFilePath, std::string outputFilePath)
	{
		m_FileReader.open(inputFilePath, std::ios::in);
		if (m_FileReader.fail())
		{
			std::cerr<< "Error: Couldnt open input file " << inputFilePath << std::endl;
			exit(EXIT_FAILURE);
		}
		m_FileWriter.open(outputFilePath, std::ios::out);
		if (m_FileWriter.fail())
		{
			std::cerr << "Error: Couldnt open output file " << outputFilePath << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	inline ~RBTreeFileHandler()
	{
		m_FileReader.close();
		m_FileWriter.close();
	}

	void ExecComands()
	{
		std::string line;
		while (std::getline(m_FileReader, line))
		{
			std::vector<std::string> tokens = SplitOnSpace(line);
			if (tokens.size() == 0)
				break;

			if (tokens.front() == "INC")
			{
				if (tokens.size() != 2)
				{
					std::cerr << "Error: INC command requires 2 argument" << std::endl;
					return;
				}

				int key = std::stoi(tokens[1]);
				m_Tree.Insert(key);
			}
			else if (tokens.front() == "REM")
			{
				if (tokens.size() != 2)
				{
					std::cerr << "Error: REM command requires 2 argument" << std::endl;
					return;
				}

				int key = std::stoi(tokens[1]);
				m_Tree.Remove(key);
			}
			else if (tokens.front() == "SUC")
			{
				if (tokens.size() != 3)
				{
					std::cerr << "Error: SUC command requires 2 arguments" << std::endl;
					return;
				}

				int key = std::stoi(tokens[1]);
				int version = std::stoi(tokens[2]);
				int successor = m_Tree.Successor(key, version);
				m_FileWriter << (successor == INT32_MAX ? "Infinito" : std::to_string(successor)) << '\n';
			}
			else if (tokens.front() == "IMP")
			{
				if (tokens.size() != 2)
				{
					std::cerr << "Error: IMP command requires 1 argument" << std::endl;
					return;
				}

				int version = std::stoi(tokens[1]);
				m_Tree.FPrint(version, m_FileWriter);
			}
			else
			{
				std::cerr << "Error: Unknown command " << tokens.front() << std::endl;
				return;
			}
		}
	}

private:
	inline std::vector<std::string> SplitOnSpace(std::string line) const
	{
		std::stringstream ss(line);
		std::string token;
		std::vector<std::string> tokens;
		while (ss >> token)
			tokens.push_back(token);

		return tokens;
	}

private:
	std::ifstream m_FileReader;
	std::ofstream m_FileWriter;

	RBTree m_Tree;
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Comand line expects 2 arguments but got " << argc - 1 << std::endl;
		std::cerr << "Usage example: RBTreeFileHandler input.txt output.txt" << std::endl;
		return EXIT_FAILURE;
	}

	RBTreeFileHandler fileHandler(argv[1], argv[2]);
	fileHandler.ExecComands();
}