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

	static int __sys_cpus = 1;

#ifdef PLATFORM_LINUX
	__attribute__((constructor)) static void __sys_cpus_init() {
		int i = sysconf(_SC_NPROCESSORS_CONF);
		__sys_cpus = i < 0 ? 1 : i;
	}
#endif

	int cpus() {
#if __cplusplus > 199711L	
		return std::thread::hardware_concurrency();
#else		
		return __sys_cpus;
#endif		
	}

	//
	// strerror
	//
	
	
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
#endif
	
	const char* strerror(int err) {
#ifdef PLATFORM_WINDOWS
		if (!__errlist) { __strerror_init(); }
#endif
		return err >= 0 && err < SYS_ERRNO ? __errlist[err] : "Unknown error";
	}


	//
	// time & timestamp
	//

	u64 timeSecond() {
		// cost of executing 1 million times is: 4 ms
		return std::time(nullptr);
	}
	
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
	const char* timestamp(char* buf, size_t len, u64 seconds, const char* time_format) {
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
		
		std::strftime(buf, len, time_format, &result);

		return (const char *) buf;
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
	// string utilities
	//
	
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
		
	bool splitString(const char* str, char cr, std::vector<int>& v) {
		char *newstr = strdup(str);
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, cr)) != nullptr) {
			*p++ = '\0';
			int value = strtol(s, (char**)NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
			v.push_back(value);
			s = p;
		}
		if (*s != '\0') {
			int value = strtol(s, (char**) nullptr, 10);
			if (errno == EINVAL || errno == ERANGE) {
				SafeFree(newstr); 
				return false;
			}
			v.push_back(value);
		}
		else {
			// Note: add default value ??
		}
		SafeFree(newstr);
		return true;
	}
		
	bool splitString(const char* str, char cr, std::vector<std::string>& v) {
		char *newstr = strdup(str);
		char *s = newstr, *p = newstr;
		while ((p = strchr(p, cr)) != nullptr) {
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
	
#if __GNUC__ >= 5
	std::wstring s2ws(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.from_bytes(str);
	}
	
	std::string ws2s(const std::wstring& wstr)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.to_bytes(wstr);
	}
#else
	std::wstring s2ws(const std::string& str) 
	{
		if (str.empty()) {
			return L"";
		}
		unsigned len = str.size() + 1;
		setlocale(LC_CTYPE, "en_US.UTF-8");
		std::unique_ptr<wchar_t[]> p(new wchar_t[len]);
		mbstowcs(p.get(), str.c_str(), len);
		std::wstring w_str(p.get());
		return w_str;
	}
	std::string ws2s(const std::wstring& w_str) 
	{
		if (w_str.empty()) {
			return "";
		}
		unsigned len = w_str.size() * 4 + 1;
		setlocale(LC_CTYPE, "en_US.UTF-8");
		std::unique_ptr<char[]> p(new char[len]);
		wcstombs(p.get(), w_str.c_str(), len);
		std::string str(p.get());
		return str;
	}	
#endif
	
	

	//
	// network address & port compose
	//

#pragma pack(1)
	union AddressUnion {
		struct {
			u32 address;
			u32 port;
		};
		u64 value;
	};
#pragma pack()

	u64 hashAddress(const char* address, int port) {
		struct in_addr inp;
		if (inet_aton(address, &inp) == 0) {
			Error.cout("Bad network address: %s", address);
			return 0;
		}
		
		AddressUnion u;
		u.address = inp.s_addr;
		u.port = htons(port);
		
		return u.value;
	}
	
	void splitAddress(u64 value, std::string& address, int& port) {
		AddressUnion u;
		u.value = value;
		
		struct in_addr inp;
		inp.s_addr = u.address;		
		address = inet_ntoa(inp);

		port = ntohs(u.port);
	}

	const char* gethostname() {
		static char __hostname[64];
		::gethostname(__hostname, sizeof(__hostname));
		return __hostname;
	}
	
	//
	// file & directory i/o
	//
	
	bool isDir(const char* file) {
		struct stat buf;
		if (stat(file, &buf) != 0) { return false; }
		return S_ISDIR(buf.st_mode);
	}

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

	const char* getDirectory(char* buf, size_t bufsize, const char* filename) {
		strncpy(buf, filename, bufsize);
		char *p = strrchr(buf, '/');
		if (p) {
			*p = '\0';
		}
		else {
			buf[0] = '.', buf[1] = '\0';
		}
		return buf;
	}

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

	bool createDirectory(const char* path) {
		char* current_dir = get_current_dir_name();
		char name[PATH_MAX], *dir = name;
		strncpy(name, path, sizeof(name));
		while (dir != nullptr) {
			char* tailer = strchr(dir, '/');
			if (tailer) {
				*tailer++ = '\0';
			}

			if (strlen(dir) == 0) {
				goto dont_need_mkdir;
			}

		 	if (access(dir, F_OK)) {
				umask(0);
				if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) { 
					Error.cout("mkdir directory:%s error: %d, %s", dir, errno, strerror(errno));
					return false;
				}
			}

			if (access(dir, X_OK)) {
				Error.cout("access directory:%s error: %d, %s", dir, errno, strerror(errno));
				return false;
			}

			if (chdir(dir)) {
				Error.cout("chdir directory:%s error: %d, %s", dir, errno, strerror(errno));
				return false;
			}

dont_need_mkdir:			
			dir = tailer;
		}

		chdir(current_dir);
		SafeFree(current_dir);

		return true;
	}
	
	u64 getFileSize(const char* filename) {
		struct stat buf;
		if (stat(filename, &buf) != 0) { return 0; }
		return buf.st_size;
	}

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
			Error.cout("loadfile exception:%s", e.what());
			return false;
		}
		
		return true;
	}
	


	//
	// limits
	//

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

	int randomValue() {
		std::random_device rd;
		return rd();
	}

	thread_local int _randomSeed = std::time(nullptr) + ::getpid();
	thread_local std::default_random_engine _randomEngine(_randomSeed);

	int getRandomSeed() {
		return _randomSeed;
	}

	int setRandomSeed(int seed) {
		_randomSeed = seed;
		_randomEngine.seed(_randomSeed);
		return _randomSeed;
	}

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
	void randomString(std::string& result, size_t len, bool enable_digital, bool enable_lower, bool enable_upper) {
		while (len > 0 && (enable_digital || enable_lower || enable_upper)) {
			int i = randomBetween(0, sizeof(_alphabet) - 1);
			assert(i >= 0 && i < (int)sizeof(_alphabet));
			char c = _alphabet[i];
			if ((enable_digital && std::isdigit(c)) 
					|| (enable_lower && std::islower(c))
					|| (enable_upper && std::isupper(c))) {
				--len;
				result.push_back(c);
			}
		}
	}

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
	// misc
	//

	bool isdigit(const std::string& s) {
		return !s.empty() && std::find_if(s.begin(), 
			s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
	}

	s64 threadid() {
		std::stringstream ss;
		ss << std::this_thread::get_id();
		return std::stol(ss.str().c_str());
	}

	bool isInteger(double value) {
		return value == (int64_t)value;
	}

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

		Trace << "StackSize: " << getStackSizeLimit() << ", MaxOpenFiles: " << getOpenFilesLimit();		
		
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
			Trace << "shard: not configured";
		}
		
		//
		// output 3rd libraries
		//
		Trace.cout("all 3rd libraries:");
		Trace.cout("    libbundle: %d.%d.%d", BUNDLE_VERSION_MAJOR, BUNDLE_VERSION_MINOR, BUNDLE_VERSION_PATCH);
		
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

	void shutdown_bundle_library() {
		//NOTE: cleanup internal resource
		Trace.cout("shutdown bundle library with terminate reason: %d", sConfig.terminate_reason);
		Easylog::syslog()->stop();
		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();
	}

	void* memdup(void* buffer, size_t size) {
		void* newbuffer = ::malloc(size);
		memcpy(newbuffer, buffer, size);
		return newbuffer;
	}

	static std::vector<std::string> __argv;
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
}

