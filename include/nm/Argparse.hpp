#ifndef NM_ARGPARSE_HPP
#define NM_ARGPARSE_HPP

#include <getopt.h>
#include <unordered_map>

namespace nm
{
	typedef std::unordered_map<std::string, std::string> options_map;
	static struct option long_options[] =
	{
		{"config_file", required_argument, 0, 'c'},
		{"server", no_argument, 0, 's'},
		{"port", required_argument, 0, 'p'},
		{"log", required_argument, 0, 'l'},
		{"help", no_argument, 0, 'h'},
		{"game", required_argument, 0, 'g'},
		{0, 0, 0, 0}
	};

	options_map parse_options(int argc, char* argv[]);
}

#endif //NM_ARG_PARSE_HPP
