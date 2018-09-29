/*
 * \file: Tools.h
 * \brief: Created by hushouguo at Jul 07 2017 00:17:22
 */
 
#ifndef __TOOLS_H__
#define __TOOLS_H__

BEGIN_NAMESPACE_BUNDLE {

	int cpus();
	const char* strerror(int err);


	// time
	u64 timeSecond();
	u64 timeMillisecond();
	const char* timestamp(char* buf, size_t len, u64 seconds = 0, const char* time_format = nullptr);


	// string
	u32 hashString(const char* s);
	u32 hashString(const char* s, size_t len);
	u32 hashString(const std::string& s);
	bool splitString(const char* str, char cr, std::vector<int>& v);
	bool splitString(const char* str, char cr, std::vector<std::string>& v);
	std::wstring s2ws(const std::string& str);
	std::string ws2s(const std::wstring& w_str);
	

	// network
	u64 hashAddress(const char* address, int port);
	void splitAddress(u64 value, std::string& address, int& port);
	const char* gethostname();


	// file & directory i/o
	bool existDir(const char* file);
	bool isDir(const char* file);
	bool accessableDir(const char* file);
	bool readableDir(const char* file);
	bool writableDir(const char* file);
	const char* getDirectory(char* buf, size_t bufsize, const char* filename);
	bool traverseDirectory(const char* folder, const char* filter_suffix, std::function<bool(const char*)>& callback);
	bool createDirectory(const char* path);
	u64 getFileSize(const char* filename);
	bool loadfile(const char* filename, std::string& s);
	


	// limits
	bool setStackSizeLimit(u32 value);
	u32 getStackSizeLimit();
	bool setOpenFilesLimit(u32 value);
	u32 getOpenFilesLimit();


	// random & random_between
	int randomValue();
	int getRandomSeed();
	int setRandomSeed(int seed);
	int randomBetween(int min, int max);
	float randomBetween(float min, float max);
	double randomBetween(double min, double max);
	void randomString(std::string& result, size_t len, bool enable_digital, bool enable_lower, bool enable_upper);


	// misc
	bool isdigit(const std::string& s);
	s64 threadid();	
	bool isInteger(double value);
	bool isUTF8String(const std::string& string);
	const char* signalString(int sig);
	void* memdup(void* buffer, size_t size);
	void setProcesstitle(int argc, char* argv[], const char* title);
	void resetProcesstitle(int argc, char* argv[]);
	const char* getProgramName();
	template <typename HANDLER>
	void setSignal(int sig, HANDLER handler) {
		struct sigaction act;
		// `sa_handler` will not take effect if it is not set
		// default action:
		// abort: SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,SIGXCPU,SIGXFSZ
		// exit: SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,SIGPROF,SIGSYS,SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM
		// stop: SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU
		// default ignore: SIGCHLD,SIGPWR,SIGURG,SIGWINCH
		// Don't call Non reentrant function, just like malloc, free etc, i/o function also cannot call.
        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
        sigaction(sig, &act, nullptr);
	}
	bool init_runtime_environment(int argc, char* argv[]);
	void shutdown_bundle_library();


	// This function does not distinguish between a missing key and a key mapped
	// to a NULL value.
	template <class Collection>
		typename Collection::value_type::second_type
		FindOrNull(const Collection& collection, const typename Collection::value_type::first_type& key) 
		{
			typename Collection::const_iterator i = collection.find(key);
			return i == collection.end() ? typename Collection::value_type::second_type() : i->second;
		}

	// Same as above, except takes non-const reference to collection.
	template <class Collection>
		typename Collection::value_type::second_type
		FindOrNull(Collection& collection, const typename Collection::value_type::first_type& key) 
		{
			typename Collection::iterator i = collection.find(key);
			return i == collection.end() ? typename Collection::value_type::second_type() : i->second;
		}


	// Returns true if and only if the given collection contains the given key.
	template <class Collection, class Key>
		bool ContainsKey(const Collection& collection, const Key& key) {
			return collection.find(key) != collection.end();
		}

	// Returns a pointer to the const value associated with the given key if it
	// exists, or NULL otherwise.
	template <class Collection>
		const typename Collection::value_type::second_type*
		FindPtrOrNull(const Collection& collection, const typename Collection::value_type::first_type& key) 
		{
			typename Collection::const_iterator i = collection.find(key);
			return i == collection.end() ? nullptr : &i->second;
		}

	// Same as above but returns a pointer to the non-const value.
	template <class Collection>
		typename Collection::value_type::second_type*
		FindPtrOrNull(Collection& collection, const typename Collection::value_type::first_type& key) 
		{
			typename Collection::iterator i = collection.find(key);
			return i == collection.end() ? nullptr : &i->second;
		}
	
}

#endif
