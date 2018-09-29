/*
 * \file: Tools.h
 * \brief: Created by hushouguo at Jul 07 2017 00:17:22
 */
 
#ifndef __TOOLS_H__
#define __TOOLS_H__

BEGIN_NAMESPACE_BUNDLE {

	//
	// get the number of cpu
	int cpus();

	//
	// like ::strerror
	const char* strerror(int err);

	//
	// get current time seconds
	u64 timeSecond();

	//
	// get current time milliseconds
	u64 timeMillisecond();
	
	//
	// get current timestamp
	//	if time_format is nullptr, default value is "%y/%02m/%02d %02H:%02M:%02S", like: 18/06/29 15:04:18
	const char* timestamp(char* buffer, size_t len, u64 seconds = 0, const char* time_format = nullptr);


	//
	// hash string
	u32 hashString(const char* s);
	u32 hashString(const char* s, size_t len);
	u32 hashString(const std::string& s);

	//
	// extrace string to int, long, long long or string by specifying seperate character
	bool splitString(const char* cstr, char sc, std::vector<int>& v);
	bool splitString(const char* cstr, char sc, std::vector<long>& v);
	bool splitString(const char* cstr, char sc, std::vector<long long>& v);
	bool splitString(const char* cstr, char sc, std::vector<std::string>& v);

	//
	// string and wstring convert each other
	std::wstring string2wstring(const std::string& s);
	std::string wstring2string(const std::wstring& ws);
	

	//
	// network address and u64 convert each other
	u64 combineNetworkEndpoint(const char* address, int port);
	std::tuple<std::string, int> splitNetworkEndpoint(u64 value);

	//
	// gethostname c function simple wrapping
	const char* gethostname();

	//
	// get current worker absolute directory, like: /home/hushouguo/workspace
	const char* getCurrentDirectory();
	
	//
	// extract dir from fullname
	//	path: ./workspace/test.cpp
	//	return: ./workspace
	const char* getDirectoryName(const char* fullname);

	//
	// extract filename from a path
	//	path: ./workspace/test.cpp
	//	return: test.cpp
	const char* getFilename(const char* fullname);
	
	//
	// convert to absolute directory
	// 	like: ./workspace 			=> /home/hushouguo/workspace
	//	like: ./workspace/test.cpp 	=> /home/hushouguo/workspace/test.cpp
	const char* absoluteDirectory(const char* fullname);

	//
	// test for the file is a directory
	bool isDir(const char* file);

	//
	// existDir: 
	//	test for the existence of the file
	// accessableDir, readableDir, writableDir:
	// 	test whether the file exists and grants read, write, and execute permissions, respectively.
	bool existDir(const char* file);
	bool accessableDir(const char* file);
	bool readableDir(const char* file);
	bool writableDir(const char* file);

	//
	// create inexistence folder
	bool createDirectory(const char* path);

	//
	// iterate specifying folder
	bool traverseDirectory(const char* path, const char* filter_suffix, std::function<bool(const char*)>& cb);

	//
	// get existence file size
	u64 getFileSize(const char* filename);

	//
	// load file content into string
	bool loadfile(const char* filename, std::string& s);
	
	//
	// limits: stack_size, max_files
	bool setStackSizeLimit(u32 value);
	u32 getStackSizeLimit();
	bool setOpenFilesLimit(u32 value);
	u32 getOpenFilesLimit();


	//
	// get a random value
	int randomValue();

	//
	// set/get random seed
	int getRandomSeed();
	int setRandomSeed(int seed);

	//
	// random between int, long, long long, float or double, [min, max]
	int randomBetween(int min, int max);
	long randomBetween(long min, long max);
	long long randomBetween(long long min, long long max);
	float randomBetween(float min, float max);
	double randomBetween(double min, double max);
	
	//
	// random a string
	void randomString(std::string& result, size_t len, bool has_digit, bool has_lowercase, bool has_uppercase);


	//
	// check a string is all numeric
	bool isDigit(const std::string& s);

	//
	// get current thread id
	s64 threadid();	

	//
	// check that a floating point number is integer
	bool isInteger(double value);

	//
	// check that a string is utf8 encoding
	bool isUTF8String(const std::string& s);

	//
	// signal value to string
	const char* signalString(int sig);

	//
	// allocate new buffer and copy buffer to new buffer, like: strdup
	void* memdup(void* buffer, size_t len);

	//
	// setup/reset process title
	void setProcesstitle(int argc, char* argv[], const char* newtitle);
	void resetProcesstitle(int argc, char* argv[]);

	
	//
	// get the execution of the program, like: foo
	const char* getProgramName();
	
	//
	// get the complete execution of the program, like: ./bin/foo
	const char* getProgramFullName();


	//
	// init and shutdown library
	bool init_runtime_environment(int argc, char* argv[]);
	void shutdown_bundle_library();

	//
	// install signal handler
	template <typename HANDLER>
	void setSignal(int sig, HANDLER handler) {
		struct sigaction act;
		// `sa_handler` will not take effect if it is not set
		// default action:
		// abort: SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,SIGXCPU,SIGXFSZ
		// exit: SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,SIGPROF,SIGSYS,SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM
		// stop: SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU
		// default ignore: SIGCHLD,SIGPWR,SIGURG,SIGWINCH
		//
		// Don't call Non reentrant function, just like malloc, free etc, i/o function also cannot call.
        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
        act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
        sigaction(sig, &act, nullptr);
	}


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
