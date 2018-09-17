/*
 * \file: Config.cpp
 * \brief: Created by hushouguo at 01:53:43 Aug 31 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	bool Config::loadconf(const char* xmlfile) {
		XmlParser xmlParser;
		if (!xmlParser.open(xmlfile)) {
			fprintf(stderr, "loadconf: %s failure", xmlfile);
			return false;
		}

		this->clear();
		xmlParser.makeRegistry(this);
		xmlParser.final();
		this->dump();
		return true;			
	}

	bool Config::init(int argc, char* argv[]) {
		int c;
		bool guard = false;
		std::string confile;		
		/*--argc, argv++;*/
		while ((c = getopt(argc, argv, "dDhHc:g")) != -1) {
			switch (c) {
				case 'd': case 'D': sConfig.runasdaemon = true; break;
				case 'c': confile = optarg; break;
				case 'g': guard = true; break;
				case 'h': case 'H': default: 
					fprintf(stderr, "Usage: executable [OPTIONS]\n");
					fprintf(stderr, "    OPTIONS:\n");
					fprintf(stderr, "      -d: 			run as daemon, default: %s\n", sConfig.runasdaemon ? "true" : "false");
					fprintf(stderr, "      -c filename: load config file\n");
					fprintf(stderr, "      -g: 			enable guard process\n");
					return false;
			}
		}

		setlocale(LC_ALL, "");// for chinese console output

		if (runasdaemon) {
			daemon(1, 1); /* nochdir, noclose */
			if (guard) {
				while (!sConfig.halt) {
					int pid = fork();
					if (pid) {
						setProcesstitle(argc, argv, " guard");
						int status = 0;
						waitpid(pid, &status, 0);
						if (status == 1) {
							fprintf(stderr, "status: %d\n", status);
							::exit(0);	// status == 1 means childProcess exit on init stage
						}
						resetProcesstitle(argc, argv);
					}
					else { break; }
				}
			}
		}

		return confile.empty() ? true : this->loadconf(confile.c_str());
	}

	INITIALIZE_INSTANCE(Config);
}
