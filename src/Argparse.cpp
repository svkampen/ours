#include <nm/Argparse.hpp>

extern const char* const VERSION;

namespace nm
{
	std::unordered_map<std::string, std::string> parse_options(int argc, char* argv[])
	{

		std::unordered_map<std::string, std::string> options =
		{
			{"config_file", ""},
			{"log_file", "netmine.log"},
			{"default_save_path", "./save.nm"},
			{"game", ""},
			{"port", "4096"}
		};

		while (true)
		{
			int option_index = 0;
			int c = getopt_long(argc, argv, "c:shp:l:",
					long_options, &option_index);

			if (c == -1)
				break;

			switch (c)
			{
				case 'l':
					options["log_file"] = optarg;
					break;
				case 'h':
					printf("netmine %s - copyright (c) 2016 Sam van Kampen\n", VERSION);
					printf("usage: netmine [-slpcgh]\n");
					printf("  -s\t\tTurn on server mode\n");
					printf("  -l\t\tSpecify a different log file (default: netmine.log)\n");
					printf("  -p [port]\tSpecify a different port (default: 51745)\n");
					printf("  -c [file]\tSpecify a different config file (default: ~/.nmrc)\n");
					printf("  -g [file]\tSpecify a game to load.\n");
					printf("  -h\t\tDisplay this help message\n");
					exit(0);
				case 'g':
					options["game"] = optarg;
					break;
				case 'c':
					options["config_file"] = optarg;
					break;
				case 's':
					options["server"] = "true";
					break;
				case 'p':
					options["port"] = optarg;
					break;
				case '?':
					exit(1);
					break; // getopt already prints an error message
				default:
					abort();
			}
		}

		if (optind < argc)
			options["rest"] = argv[optind];
		return options;
	}
}
