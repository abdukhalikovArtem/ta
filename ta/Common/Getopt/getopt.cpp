#include "getopt.hpp"

std::vector<std::pair<std::string, std::string>>  getopt(int argc, char **argv) {
	std::string arg, cmd;
	//std::pair<std::string, std::string> packet;
	std::vector<std::pair<std::string, std::string>> result;
	if (argc % 2 == 0) // кол-во аргументов нечетно
	{
		std::cerr << "Wrong arguments number must be even " <<  std::endl;
		exit(1);
	}
	for (int i = 1; i < (argc - 1); i+=2) {
		if (argv[i][0] == '-') {
			cmd = std::string(&argv[i][1]);
			arg = std::string(&argv[i + 1][0]);
			auto packet = std::pair<std::string, std::string>(cmd,arg);
			result.push_back(packet);
		}
		else {
			std::cerr << "Wrong argument " << std::string(&argv[i][0]) << std::endl;
			exit(1);
		}

	}

//	Log for test
//	for (auto& a : result)
//		std::cout << "cmd : " << a.first << "; arg : " << a.second << std::endl;

	return result;
}
