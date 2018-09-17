/*
 * \file: Config.h
 * \brief: Created by hushouguo at 01:21:11 Aug 31 2018
 */
 
#ifndef __CONFIG_H__
#define __CONFIG_H__

BEGIN_NAMESPACE_BUNDLE {
	class Config : public Registry {
		public:
			bool halt = false;
			bool runasdaemon = false;
			inline void syshalt() { this->halt = true; }
			bool init(int argc, char* argv[]);

		private:
			bool loadconf(const char* xmlfile);
	};
}

#define sConfig bundle::Singleton<bundle::Config>::getInstance()

#endif
