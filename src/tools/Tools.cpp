/*
 * \file: Tools.cpp
 * \brief: Created by hushouguo at Jul 07 2017 00:17:25
 */

#include "bundle.h"

#ifdef PLATFORM_WINDOWS
#pragma warning(disable:4996) // disable strdup warning
#endif

#define CONVERT_CST_TIME

BEGIN_NAMESPACE_BUNDLE {

	static u32 __sys_cpus = 1;

#ifdef PLATFORM_LINUX
	__attribute__((constructor)) static void __sys_cpus_init() {
		int i = sysconf(_SC_NPROCESSORS_CONF);
		__sys_cpus = i < 0 ? 1 : i;
	}
#endif

	//
	// get the number of cpu
	u32 cpus() {
#if __cplusplus > 199711L	
		return std::thread::hardware_concurrency();
#else		
		return __sys_cpus;
#endif
	}


#define SYS_ERRNO	128

	static char** __errlist = nullptr;

#ifdef PLATFORM_LINUX
	__attribute__((constructor)) static void __strerror_init() {
		if (!__errlist) {
			__errlist = (char **) ::malloc(SYS_ERRNO * sizeof(char*));
		}
		for (int err = 0; err < SYS_ERRNO; ++err) {
			__errlist[err] = strdup(::strerror(err));
		}
	}

	__attribute__((destructor)) static void __strerror_destroy() {
		if (__errlist) {
			for (int err = 0; err < SYS_ERRNO; ++err) {
				SafeFree(__errlist[err]);
			}
			SafeFree(__errlist);
		}
	}
#endif

	//
	// like ::strerror
	const char* strerror(int err) {
#ifdef PLATFORM_WINDOWS
		if (!__errlist) { __strerror_init(); }
#endif
		return err >= 0 && err < SYS_ERRNO ? __errlist[err] : ::strerror(err);
	}


	//
	// get current time seconds	
	u64 timeSecond() {		
		return std::time(nullptr); // cost of executing 1 million times is: 4 ms
	}

	//
	// get current time milliseconds	
	u64 timeMillisecond() {
		// cost of executing 1 million times is:
		// 		c++ 11 waste: 38 ms
		//		gettimeofday waste: 19 ms
#if true
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif		
	}

	// cost of executing 1 million times is:
	//		c++ 11 waste: 1721 ms
	//		gettimeofday waste: 138 ms
#if true
	//
	// get current timestamp
	//	if time_format is nullptr, default value is "%y/%02m/%02d %02H:%02M:%02S", like: 18/06/29 15:04:18
	const char* timestamp(char* buffer, size_t len, u64 seconds, const char* time_format) {
		struct timeval tv = { tv_sec: (time_t) seconds, tv_usec: 0 };
		if (tv.tv_sec == 0) {
			gettimeofday(&tv, nullptr);
		}

		if (!time_format) {
			time_format = "%y/%02m/%02d %02H:%02M:%02S"; // 18/06/29 15:04:18
		}

#ifdef CONVERT_CST_TIME
		// utc -> cst
		tv.tv_sec += 8 * 3600;
#endif

		struct tm result;
		gmtime_r(&tv.tv_sec, &result);

		std::strftime(buffer, len, time_format, &result);

		return (const char *) buffer;
	}
#else	
	const char* timestamp(char* buf, size_t len, u64 milliseconds, const char* time_format, bool with_milliseconds) {
		if (!time_format) {
			time_format = "%y/%02m/%02d %02H:%02M:%02S"; // 18/06/29 15:04:18
		}

		std::time_t seconds;
		std::time_t ts_milliseconds;
		if (milliseconds == 0) {
			auto time_now = std::chrono::system_clock::now();
			auto duration_millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
			auto millisecond_part = duration_millisecond - std::chrono::duration_cast<std::chrono::seconds>(duration_millisecond);
			seconds = std::chrono::system_clock::to_time_t(time_now);
			ts_milliseconds = millisecond_part.count();
			milliseconds = seconds * 1000 + ts_milliseconds;
		}
		else {
			seconds = milliseconds / 1000;
			ts_milliseconds = milliseconds % 1000;
		}

		//Note: for GCC 5, std::get_time or std::put_time available
		//std::locale::global(std::locale("ja_JP.utf8"));
		//std::tm local_time_now = * std::localtime(&seconds);
		size_t n = std::strftime(buf, len, time_format, std::localtime(&seconds));
		if (with_milliseconds) {
			std::snprintf(&buf[n], len - n, "|%03ld", ts_milliseconds);
		}

		return (const char *) buf;	
	}
#endif


	//
	// hash string
	u32 hashString(const char* s) {
		return hashString(s, strlen(s));
	}		
	u32 hashString(const char* s, size_t len) {
#ifdef USE_CPP_HASH
		return std::hash<std::string>{}(s);
#else
		u32 h = 0, g;
		const char* end = s + len;
		while (s < end) {
			h = (h << 4) + *s++;
			if ((g = (h & 0xF0000000))) {
				h = h ^ (g >> 24);
				h = h ^ g;
			}
		}
		return h;
#endif
	}		
	u32 hashString(const std::string& s) {
		return hashString(s.data(), s.length());
	}


	//
	// extrace string to int, long, long long or string by specifying seperate character
	bool splitString(const char* cstr, char sc, std::vector<int>& v) {
		char* newstr = strdup(cstr);
		int value = 0;
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, sc)) != nullptr) {
			*p++ = '\0';
#if true
			std::string ss = s;
			try {
				value = std::stoi(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtol(s, (char**)NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif			
			v.push_back(value);
			s = p;
		}

		if (*s != '\0') {
#if true		
			std::string ss = s;
			try {
				value = std::stoi(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtol(s, (char**) nullptr, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif
			v.push_back(value);
		}
		else {
			// Note: add default value ??
		}
		SafeFree(newstr);
		return true;
	}

	bool splitString(const char* cstr, char sc, std::vector<long>& v) {
		char* newstr = strdup(cstr);
		int value = 0;
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, sc)) != nullptr) {
			*p++ = '\0';
#if true
			std::string ss = s;
			try {
				value = std::stol(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtol(s, (char**)NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif			
			v.push_back(value);
			s = p;
		}

		if (*s != '\0') {
#if true		
			std::string ss = s;
			try {
				value = std::stol(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtol(s, (char**) nullptr, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif
			v.push_back(value);
		}
		else {
			// Note: add default value ??
		}
		SafeFree(newstr);
		return true;
	}

	bool splitString(const char* cstr, char sc, std::vector<long long>& v) {
		char* newstr = strdup(cstr);
		int value = 0;
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, sc)) != nullptr) {
			*p++ = '\0';
#if true
			std::string ss = s;
			try {
				value = std::stoll(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtoll(s, (char**)NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif			
			v.push_back(value);
			s = p;
		}

		if (*s != '\0') {
#if true		
			std::string ss = s;
			try {
				value = std::stoll(ss);
			} catch(...) {
				SafeFree(newstr);
				return false;
			}
#else
			value = strtoll(s, (char**) nullptr, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
#endif
			v.push_back(value);
		}
		else {
			// Note: add default value ??
		}
		SafeFree(newstr);
		return true;
	}


	bool splitString(const char* cstr, char sc, std::vector<std::string>& v) {
		char *newstr = strdup(cstr);
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, sc)) != nullptr) {
			*p++ = '\0';
			v.push_back(s);
			s = p;
		}
		if (*s != '\0') {
			v.push_back(s);
		}
		else {
			v.push_back("");
		}
		SafeFree(newstr);
		return true;
	}

	//
	// string and wstring convert each other

#if __GNUC__ >= 5
	std::wstring string2wstring(const std::string& s)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.from_bytes(s);
	}

	std::string wstring2string(const std::wstring& ws)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.to_bytes(ws);
	}
#else
	std::wstring string2wstring(const std::string& s) 
	{
		if (s.empty()) {
			return L"";
		}
		unsigned len = s.size() + 1;
		setlocale(LC_CTYPE, "en_US.UTF-8");
		std::unique_ptr<wchar_t[]> p(new wchar_t[len]);
		mbstowcs(p.get(), s.c_str(), len);
		std::wstring ws(p.get());
		return ws;
	}
	std::string wstring2string(const std::wstring& ws) 
	{
		if (ws.empty()) {
			return "";
		}
		unsigned len = ws.size() * 4 + 1;
		setlocale(LC_CTYPE, "en_US.UTF-8");
		std::unique_ptr<char[]> p(new char[len]);
		wcstombs(p.get(), ws.c_str(), len);
		std::string s(p.get());
		return s;
	}	
#endif

#pragma pack(1)
	union NetworkEndpoint {
		struct {
			u32 address;
			u32 port;
		};
		u64 value;
	};
#pragma pack()

	//
	// network address and u64 convert each other
	u64 combineNetworkEndpoint(const char* address, int port) {
		struct in_addr inp;
		CHECK_RETURN(inet_aton(address, &inp), 0, "Bad network address: %s", address);
		NetworkEndpoint u;
		u.address = inp.s_addr;
		u.port = htons(port);		
		return u.value;
	}

	std::tuple<std::string, int> splitNetworkEndpoint(u64 value) {
		NetworkEndpoint u;
		u.value = value;		
		struct in_addr inp;
		inp.s_addr = u.address;
		return std::make_tuple(inet_ntoa(inp), ntohs(u.port));
	}

	//
	// gethostname c function simple wrapping
	const char* gethostname() {
		static char __hostname[64];
		::gethostname(__hostname, sizeof(__hostname));
		return __hostname;
	}


	//
	// test for the file is a directory
	bool isDir(const char* file) {
		struct stat buf;
		if (stat(file, &buf) != 0) { return false; }
		return S_ISDIR(buf.st_mode);
	}

	//
	// existDir: 
	//	test for the existence of the file
	// accessableDir, readableDir, writableDir:
	// 	test whether the file exists and grants read, write, and execute permissions, respectively.
	bool existDir(const char* file) {
		return access(file, F_OK) == 0;
	}
	bool readableDir(const char* file) {
		return access(file, R_OK) == 0;
	}
	bool writableDir(const char* file) {
		return access(file, W_OK) == 0;
	}
	bool accessableDir(const char* file) {
		return access(file, X_OK) == 0;
	}

	//
	// create inexistence folder
	bool createDirectory(const char* path) {
		std::string fullPath = absoluteDirectory(path);
		std::string::size_type i = 0;
		umask(0);
		while (i < fullPath.length()) {
			std::string::size_type head = fullPath.find('/', i);
			std::string dir;
			dir.assign(fullPath, 0, head == std::string::npos ? fullPath.length() : head);
			if (!dir.empty()) {
				int rc = mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
				CHECK_RETURN(rc == 0 || errno == EEXIST, false, "mkdir:%s error:%d,%s", dir.c_str(), errno, strerror(errno));
			}
			if (head == std::string::npos) {
				break;
			}
			i = head + 1;
		}
		return true;
	}


	//
	// iterate specifying folder
	bool traverseDirectory(const char* folder, const char* filter_suffix, std::function<bool(const char*)>& callback) {
		if (!isDir(folder)) {
			return callback(folder);
		}

		DIR* dir = opendir(folder);

		struct dirent* ent;
		while ((ent = readdir(dir)) != nullptr) {
			if (ent->d_name[0] == '.') { continue; } //filter hide file

			if (filter_suffix != nullptr) {
				char* suffix = strrchr(ent->d_name, '.');//filter not .proto suffix file 
				if (!suffix || strcasecmp(suffix, filter_suffix) != 0) {
					continue; 
				}
			}

			char fullname[PATH_MAX];
			snprintf(fullname, sizeof(fullname), "%s/%s", folder, ent->d_name);
			if (ent->d_type & DT_DIR) {
				return traverseDirectory(fullname, filter_suffix, callback);
			}
			else {
				if (!callback(fullname)) { return false; }
			}
		}

		return true;
	}


	//
	// get existence file size
	u64 getFileSize(const char* filename) {
		struct stat buf;
		if (stat(filename, &buf) != 0) { return 0; }
		return buf.st_size;
	}

	//
	// load file content into string
	bool loadfile(const char* filename, std::string& s) {
		try {
#if false
			std::ifstream in(filename);
			std::isreambuf_iterator<char> begin(in);
			std::isreambuf_iterator<char> end;
			s(begin, end);
#else
			std::ifstream in(filename);
			std::ostringstream ss;
			ss << in.rdbuf();
			s = ss.str();
#endif
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "loadfile exception:%s", e.what());
		}		
		return true;
	}



	//
	// limits: stack_size, max_files

	// limits, linux default stack size: 8M (soft), 4G (hard)
	bool setStackSizeLimit(u32 value) {
		struct rlimit limit;
		int rc = getrlimit(RLIMIT_STACK, &limit);
		CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, strerror(errno));
		limit.rlim_cur = value;
		rc = setrlimit(RLIMIT_STACK, &limit);
		CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, strerror(errno));
		return true;
	}

	u32 getStackSizeLimit() {
		struct rlimit limit;
		int rc = getrlimit(RLIMIT_STACK, &limit);
		CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, strerror(errno));
		return limit.rlim_cur;
	}

	// limits, linux default open files: 1024 (soft), 4096 (hard)
	bool setOpenFilesLimit(u32 value) {
		struct rlimit limit;
		int rc = getrlimit(RLIMIT_NOFILE, &limit);
		CHECK_RETURN(rc == 0, false, "getrlimit error:%d,%s", errno, strerror(errno));
		limit.rlim_cur = value;
		rc = setrlimit(RLIMIT_NOFILE, &limit);
		CHECK_RETURN(rc == 0, false, "setrlimit error:%d,%s", errno, strerror(errno));
		return true;
	}

	u32 getOpenFilesLimit() {
		struct rlimit limit;
		int rc = getrlimit(RLIMIT_NOFILE, &limit);
		CHECK_RETURN(rc == 0, 0, "getrlimit error:%d,%s", errno, strerror(errno));
		return limit.rlim_cur;
	}


	//
	// random & random_between
	//

	//
	// get a random value
	int randomValue() {
		std::random_device rd;
		return rd();
	}

	thread_local int _randomSeed = std::time(nullptr) + ::getpid();
	thread_local std::default_random_engine _randomEngine(_randomSeed);

	//
	// set/get random seed
	int getRandomSeed() {
		return _randomSeed;
	}
	int setRandomSeed(int seed) {
		_randomSeed = seed;
		_randomEngine.seed(_randomSeed);
		return _randomSeed;
	}


	//
	// random between int, long, long long, float or double, [min, max]
	int randomBetween(int min, int max) {
		//std::default_random_engine randomEngine(_randomSeed);
		std::uniform_int_distribution<int> dist(min, max);//[min, max]
		return dist(_randomEngine);
	}

	float randomBetween(float min, float max) {
		std::uniform_real_distribution<float> dist(min, std::nextafter(max, DBL_MAX));//[min, max]
		return dist(_randomEngine);
	}

	double randomBetween(double min, double max) {
		//std::default_random_engine randomEngine(_randomSeed);
		std::uniform_real_distribution<double> dist(min, std::nextafter(max, DBL_MAX));//[min, max]
		return dist(_randomEngine);
	}

	static char _alphabet[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
	};

	//
	// random a string
	void randomString(std::string& result, size_t len, bool has_digit, bool has_lowercase, bool has_uppercase) {
		while (len > 0 && (has_digit || has_lowercase || has_uppercase)) {
			int i = randomBetween(0, sizeof(_alphabet) - 1);
			assert(i >= 0 && i < (int)sizeof(_alphabet));
			char c = _alphabet[i];
			if ((has_digit && std::isdigit(c)) 
					|| (has_lowercase && std::islower(c))
					|| (has_uppercase && std::isupper(c))) {
				--len;
				result.push_back(c);
			}
		}
	}

	//
	// signal value to string
	const char* signalString(int sig)
	{
		static const char* __sig_string[] = {
			[0] = "NULL",
			[SIGHUP] = "SIGHUP",
			[SIGINT] = "SIGINT",
			[SIGQUIT] = "SIGQUIT",
			[SIGILL] = "SIGILL",
			[SIGTRAP] = "SIGTRAP",

			[SIGABRT] = "SIGABRT",
			[SIGBUS] = "SIGBUS",
			[SIGFPE] = "SIGFPE",
			[SIGKILL] = "SIGKILL",
			[SIGUSR1] = "SIGUSR1",

			[SIGSEGV] = "SIGSEGV",
			[SIGUSR2] = "SIGUSR2",
			[SIGPIPE] = "SIGPIPE",
			[SIGALRM] = "SIGALRM",
			[SIGTERM] = "SIGTERM",

			[SIGSTKFLT] = "SIGSTKFLT",
			[SIGCHLD] = "SIGCHLD",
			[SIGCONT] = "SIGCONT",
			[SIGSTOP] = "SIGSTOP",
			[SIGTSTP] = "SIGTSTP",

			[SIGTTIN] = "SIGTTIN",
			[SIGTTOU] = "SIGTTOU",
			[SIGURG] = "SIGURG",
			[SIGXCPU] = "SIGXCPU",
			[SIGXFSZ] = "SIGXFSZ",

			[SIGVTALRM] = "SIGVTALRM",
			[SIGPROF] = "SIGPROF",
			[SIGWINCH] = "SIGWINCH",
			[SIGIO] = "SIGIO",
			[SIGPWR] = "SIGPWR",

			[SIGSYS] = "SIGSYS",
			//			[SIGRTMIN] = "SIGRTMIN",
		};
		return sig > 0 && sig < (int)(sizeof(__sig_string)/sizeof(__sig_string[0])) ? __sig_string[sig] : "null";
	}


	//
	// check a string is all numeric
	bool isDigit(const std::string& s) {
		return !s.empty() && std::find_if(s.begin(), 
				s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
	}

	//
	// get current thread id
	s64 threadid() {
		std::stringstream ss;
		ss << std::this_thread::get_id();
		return std::stol(ss.str().c_str());
	}

	//
	// check that a floating point number is integer
	bool isInteger(double value) {
		return value == (int64_t)value;
	}

	//
	// check that a string is utf8 encoding
	bool isUTF8String(const std::string& string) {
		int c,i,ix,n,j;
		for (i = 0, ix = string.length(); i < ix; i++) {
			c = (unsigned char) string[i];
			//if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
			if (0x00 <= c && c <= 0x7f) { n=0; }	// 0bbbbbbb
			else if ((c & 0xE0) == 0xC0) { n=1; }	// 110bbbbb
			else if ( c==0xed && i<(ix-1) && ((unsigned char)string[i+1] & 0xa0)==0xa0) { return false; }	//U+d800 to U+dfff
			else if ((c & 0xF0) == 0xE0) { n=2; }	// 1110bbbb
			else if ((c & 0xF8) == 0xF0) { n=3; }	// 11110bbb
			//else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
			//else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
			else { return false; }
			for (j=0; j<n && i<ix; j++) 
			{ // n bytes matching 10bbbbbb follow ?
				if ((++i == ix) || (( (unsigned char)string[i] & 0xC0) != 0x80))
				{
					return false;
				}
			}
		}
		return true;
	}

	//
	// get the execution of the program, like: foo
	const char* getProgramName() {
#if defined(__APPLE__) || defined(__FreeBSD__)
		return getprogname();	// need libbsd
#elif defined(_GNU_SOURCE)
		//extern char* program_invocation_name;			// like: ./bin/routine
		//extern char* program_invocation_short_name;	// like: routine
		return ::program_invocation_short_name;
#else
		//extern char *__progname;						// routine:  defined by the libc
		return ::__progname;
#endif
	}

	//
	// get the complete execution of the program, like: ./bin/foo
	const char* getProgramFullName() {
#if defined(_GNU_SOURCE)
		//extern char* program_invocation_name; 		// like: ./bin/routine
		//extern char* program_invocation_short_name;	// like: routine
		return ::program_invocation_name;
#else
		return ::getenv("_");
#endif
	}

	const char* getCurrentDirectory() {
		static char __dir_buffer[PATH_MAX];
		//#if defined(_GNU_SOURCE)
#if false
		// absolute path name, like: /home/hushouguo/libbundle/tests 
		const char* s = get_current_dir_name();			
		strncpy(__dir_buffer, s, sizeof(__dir_buffer));
		SafeFree(s);
		return __dir_buffer;
#else
		// absolute path name, like: /home/hushouguo/libbundle/tests
		return ::getcwd(__dir_buffer, sizeof(__dir_buffer));
#endif
	}

	const char* getDirectoryName(const char* fullname) {
		static char __dir_buffer[PATH_MAX];
		strncpy(__dir_buffer, fullname, sizeof(__dir_buffer));
		return dirname(__dir_buffer);
	}

	const char* getFilename(const char* fullname) {
		static char __filename_buffer[PATH_MAX];
		strncpy(__filename_buffer, fullname, sizeof(__filename_buffer));
		return basename(__filename_buffer);
	}

	const char* absoluteDirectory(const char* fullname) {
		static char __dir_buffer[PATH_MAX];		
		char* realdir = realpath(getDirectoryName(fullname), nullptr);
		snprintf(__dir_buffer, sizeof(__dir_buffer), "%s/%s", realdir, getFilename(fullname));
		SafeFree(realdir);
		return __dir_buffer;
	}

	bool init_runtime_environment(int argc, char* argv[]) {
		// Verify that the version of the library that we linked against is
		// compatible with the version of the headers we compiled against.
		GOOGLE_PROTOBUF_VERIFY_VERSION;

		//
		// parser command line arguments
		//
		if (!sConfig.init(argc, argv)) { return false; }

		//
		// config Easylog
		//
		Easylog::syslog()->set_level((EasylogSeverityLevel) sConfig.get("log.level", GLOBAL));
		Easylog::syslog()->set_autosplit_day(sConfig.get("log.autosplit_day", true));
		Easylog::syslog()->set_autosplit_hour(sConfig.get("log.autosplit_hour", false));
		Easylog::syslog()->set_destination(sConfig.get("log.dir", ".logs"));
		Easylog::syslog()->set_tofile(GLOBAL, getProgramName());
		Easylog::syslog()->set_tostdout(GLOBAL, sConfig.runasdaemon ? false : true);


		//
		// libbundle
		//
#ifdef DEBUG		
		Trace.cout("libbundle: %d.%d.%d, run as %s, %s, debug", BUNDLE_VERSION_MAJOR, BUNDLE_VERSION_MINOR, BUNDLE_VERSION_PATCH, sConfig.runasdaemon ? "daemon" : "console", sConfig.guard ? "with guard" : "no guard");
#else		
		Trace.cout("libbundle: %d.%d.%d, run as %s, %s, release", BUNDLE_VERSION_MAJOR, BUNDLE_VERSION_MINOR, BUNDLE_VERSION_PATCH, sConfig.runasdaemon ? "daemon" : "console", sConfig.guard ? "with guard" : "no guard");
#endif

		//
		// Config information
		//
		Trace.cout("refer to config file: %s", sConfig.confile.empty() ? "not configure" : sConfig.confile.c_str());
		sConfig.dump();


		//
		// Easylog configure information
		//
		extern const char* level_string(EasylogSeverityLevel);
		Trace.cout("Easylog:");
		Trace.cout("    log.level: %s", level_string(Easylog::syslog()->level()));
		Trace.cout("    log.autosplit_day: %s, log.autosplit_hour: %s", 
				Easylog::syslog()->autosplit_day() ? "yes" : "no", 
				Easylog::syslog()->autosplit_hour() ? "yes" : "no");
		Trace.cout("    log.dir: %s", Easylog::syslog()->destination());


		//
		// limit
		//
		size_t stack_size = sConfig.get("limit.stack_size", 0u);
		if (stack_size > 0) {
			setStackSizeLimit(stack_size);
		}

		size_t max_files = sConfig.get("limit.max_files", 0u);
		if (max_files > 0) {
			setOpenFilesLimit(max_files);
		}

		Trace.cout("stack size: %u (limit.stack_size), max files: %u (limit.max_files)", getStackSizeLimit(), getOpenFilesLimit());

		//
		// install signal handler
		//
		struct sigaction act;
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
		act.sa_handler = [](int sig) {
			// Don't call Non reentrant function, just like malloc, free etc, i/o function also cannot call.
			if (sig == SIGRTMIN) {		// SIGRTMIN: Wake up thread, nothing to do
				return;	// SIGRTMIN: #define SIGRTMIN        (__libc_current_sigrtmin ())
			}
			switch (sig) {
				case SIGHUP:			// NOTE: reload configure file
					sConfig.reload = true;
				case SIGALRM: break;	// timer expire
				default: 
					sConfig.syshalt(sig); break;
			}
		};
		sigaction(SIGHUP, &act, nullptr);		// 1
		sigaction(SIGINT, &act, nullptr);		// 2
		sigaction(SIGQUIT, &act, nullptr);		// 3
		sigaction(SIGILL, &act, nullptr);		// 4
		sigaction(SIGTRAP, &act, nullptr);		// 5
		sigaction(SIGABRT, &act, nullptr);		// 6
		sigaction(SIGIOT, &act, nullptr);		// 6
		sigaction(SIGBUS, &act, nullptr);		// 7
		sigaction(SIGFPE, &act, nullptr);		// 8
		// 9 => SIGKILL
		sigaction(SIGUSR1, &act, nullptr);		// 10
		sigaction(SIGSEGV, &act, nullptr);		// 11
		sigaction(SIGUSR2, &act, nullptr);		// 12
		sigaction(SIGPIPE, &act, nullptr);		// 13
		sigaction(SIGALRM, &act, nullptr);		// 14
		sigaction(SIGTERM, &act, nullptr);		// 15
		sigaction(SIGSTKFLT, &act, nullptr); 	// 16
		// 17 => SIGCHLD
		sigaction(SIGCONT, &act, nullptr);		// 18
		// 19 => SIGSTOP
		sigaction(SIGTSTP, &act, nullptr);		// 20
		sigaction(SIGTTIN, &act, nullptr);		// 21
		sigaction(SIGTTOU, &act, nullptr);		// 22
		sigaction(SIGURG, &act, nullptr);		// 23
		sigaction(SIGXCPU, &act, nullptr);		// 24
		sigaction(SIGXFSZ, &act, nullptr);		// 25
		sigaction(SIGVTALRM, &act, nullptr); 	// 26
		sigaction(SIGPROF, &act, nullptr);		// 27
		sigaction(SIGWINCH, &act, nullptr);		// 28
		sigaction(SIGIO, &act, nullptr); 		// 29
		sigaction(SIGPWR, &act, nullptr);		// 30
		sigaction(SIGSYS, &act, nullptr);		// 31
		sigaction(SIGRTMIN, &act, nullptr);		// 34

		//
		// output current shard
		//
		u32 shard = sConfig.get("shard.id", 0u);
		if (shard > 0) {
			Trace << "shard: " << shard;
		}
		else {
			Alarm << "shard: not configure (shard.id)";
		}

		//
		// output 3rd libraries
		//
		Trace.cout("all 3rd libraries:");

#ifdef TC_VERSION_MAJOR		
		Trace.cout("    tcmalloc: %d.%d%s", TC_VERSION_MAJOR, TC_VERSION_MINOR, TC_VERSION_PATCH);
#else
		Trace.cout("    not link tcmalloc");
#endif

#ifdef LIBEVENT_VERSION
		Trace.cout("    libevent: %s", LIBEVENT_VERSION);
#endif

#ifdef ZMQ_VERSION_MAJOR
		Trace.cout("    libzmq: %d.%d.%d", ZMQ_VERSION_MAJOR, ZMQ_VERSION_MINOR, ZMQ_VERSION_PATCH);
#endif

#ifdef LUAJIT_VERSION
		Trace.cout("    luaJIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);
#endif

#ifdef GOOGLE_PROTOBUF_VERSION
		Trace.cout("    protobuf: %d, library: %d", GOOGLE_PROTOBUF_VERSION, GOOGLE_PROTOBUF_MIN_LIBRARY_VERSION);
#endif

		Trace.cout("    rapidxml: 1.13");

#ifdef MYSQL_SERVER_VERSION		
		Trace.cout("    mysql: %s", MYSQL_SERVER_VERSION);
#endif

		Trace.cout("    gcc version: %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

		return true;
	}

	//
	// bundle library shutdown routine
	void shutdown_bundle_library() {
		//NOTE: cleanup internal resource
		Trace.cout("shutdown bundle library with terminate reason: %d", sConfig.terminate_reason);
		Easylog::syslog()->stop();
		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();
	}

	//
	// allocate new buffer and copy buffer to new buffer, like: strdup
	void* memdup(void* buffer, size_t size) {
		void* newbuffer = ::malloc(size);
		memcpy(newbuffer, buffer, size);
		return newbuffer;
	}

	static std::vector<std::string> __argv;


	//
	// setup/reset process title
	void setProcesstitle(int argc, char* argv[], const char* title) {
		if (__argv.empty()) {
			for (int i = 0; i < argc; ++i) {
				__argv.push_back(argv[i]);
			}
		}
		for (int i = 1; i < argc; ++i) {
			memset(argv[i], 0, strlen(argv[i]));
		}
		strcat(argv[0], title);
	}

	void resetProcesstitle(int argc, char* argv[]) {
		for (size_t i = 0; i < __argv.size(); ++i) {
			strcpy(argv[i], __argv[i].c_str());
		}
	}

	
	struct CurlContext {
		void* userdata;
		std::function<void(bool, std::string, std::string, void*)> callback;
		CurlContext(void* p, std::function<void(bool, std::string, std::string, void*)> func) : userdata(p), callback(func) {}
	};

	//
	//	OK:
	//	{
    //		"openid": "OPENID",
    //		"session_key": "SESSIONKEY"
	//	}
	//
	//	Error:
	//	{
    //		"errcode": 40029,
    //		"errmsg": "invalid code"
	//	}
	//
	static size_t curlWriteCallback(const char* ptr, size_t size, size_t nmemb, CurlContext* context) {
		void* userdata = context->userdata;
		std::function<void(bool, std::string, std::string, void*)> func = context->callback;
		SafeDelete(context);

		Debug << "curlWriteCallback: " << ptr;
	
		rapidjson::Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
		if (document.Parse(ptr).HasParseError()) {
			func(false, "ParseCallbackError", ptr, userdata);
			return size * nmemb;
		}

		std::string errmsg = ptr;
		if (document.HasMember("errcode")) {
			if (document.HasMember("errmsg") && document["errmsg"].IsString()) {
				errmsg = document["errmsg"].GetString();
			}
			func(false, "errcode", errmsg, userdata);
			return size * nmemb;
		}
		
		if (!document.HasMember("session_key") || document["session_key"].IsString()) {
			func(false, "not found `session_key`", errmsg, userdata);
			return size * nmemb;
		}
		
		if (!document.HasMember("openid") || !document["openid"].IsString()) {
			func(false, "not found `openid`", errmsg, userdata);
			return size * nmemb;
		}

		std::string session_key = document["session_key"].GetString();
		std::string openid = document["openid"].GetString();

		Debug << "session_key: " << session_key << ", openid: " << openid;

		func(true, session_key, openid, userdata);
		
		return size * nmemb;  
	}
	

	//
	// decode jscode to session_key & openid, session_key is empty means error happen
	bool decode_jscode(std::string appid, std::string appsecret, std::string jscode, void* userdata, std::function<void(bool, std::string, std::string, void*)> func) {
		std::ostringstream url;
		url << "https://api.weixin.qq.com/sns/jscode2session?appid=" << appid;
		url << "&secret=" << appsecret << "&js_code=" << jscode << "&grant_type=authorization_code";
		Debug << "url: " << url.str();
		CURL* easy_handle = curl_easy_init();
		CHECK_RETURN(easy_handle, false, "curl_easy_init failure");
		CurlContext* context = new CurlContext(userdata, func);
		curl_easy_setopt(easy_handle, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, nullptr);  
		curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, curlWriteCallback);
		curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, context);
		CURLcode rc = curl_easy_perform(easy_handle);
		//CHECK_RETURN(rc == CURLE_OK, false, "perform curl error: %d, %s", rc, curl_easy_strerror(rc));
		if (rc != CURLE_OK) {
			Error.cout("perform curl error: %d, %s", rc, curl_easy_strerror(rc));
			func(false, "error", curl_easy_strerror(rc), userdata);
			SafeDelete(context);
			return false;
		}		
		curl_easy_cleanup(easy_handle);
		return true;
	}
}

