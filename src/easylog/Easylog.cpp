 /*
 * \file: Easylog.cpp
 * \brief: Created by hushouguo at 17:58:34 Aug 10 2018
 */

#include "bundle.h"

//
// layout: 
//	{process}
//	{thread}
//	{user}
//	{host}
//	{level}
//	{levelshort}
//	{file}
//	{line}
//	{function}
//	{msg}
//	{datetime:%y-%m-%d}"
//	{datetime:%y/%02m/%02d %02H:%02M:%02S}
//	{millisecond:3}
// example: 
//	easylog->set_layout(GLOBAL, "[{datetime:%y/%02m/%02d %02H:%02M:%02S}|{millisecond:3}] {msg}");
//	[18/08/11 06:24:24|206] Rank capacity: 20, package: 600000
//

//
// benchmark:
//	1 million EasylogMessage object constrctor and ostringstream waste: 546 ms
//	1 million log_message but don't write to file and stdout waste: 1173 ms
//	1 million log_message and write to file stream and not flush, not stdout, waste: 1231 ms
//	1 million log_message to file and flush right now, waste: 2397 ms
//	1 million timestamp function waste: 1721 ms
//	1 million timeSecond function waste: 4 ms
//	1 million timeMillisecond function waste: 38 ms
//	1 million gettimeofday function waste: 19 ms
//	1 million gmtime to timestamp function waste: 138 ms
//

BEGIN_NAMESPACE_BUNDLE {

//
#define ENABLE_ASYNC_SEND					1
#define ENABLE_PREFIX_DATETIME				1
//#define ENABLE_PREFIX_DATETIME_LONG		1
#define ENABLE_PREFIX_DATETIME_MILLISECOND	1
#define ENABLE_PREFIX_LEVEL					1
#define ENABLE_PREFIX_ERROR_FILE			1

	const char* level_string(EasylogSeverityLevel level) {
		static const char* __level_string[] = {
			[GLOBAL]			= 	"GLOBAL",
			[LEVEL_DEBUG]		=	"DEBUG",
			[LEVEL_TRACE]		=	"TRACE",
			[LEVEL_ALARM]		=	"ALARM",
			[LEVEL_ERROR]		=	"ERROR",
			[LEVEL_PANIC]		=	"PANIC",
			[LEVEL_SYSTEM] 		=	"SYSTEM",
		};
		return __level_string[level];
	}

	const char* levelshort_string(EasylogSeverityLevel level) {
		static const char* __level_string[] = {
			[GLOBAL]			= 	"G",
			[LEVEL_DEBUG]		=	"D",	
			[LEVEL_TRACE]		=	"T",	
			[LEVEL_ALARM]		=	"ALM",
			[LEVEL_ERROR]		=	"ERR",
			[LEVEL_PANIC]		=	"PANIC",
			[LEVEL_SYSTEM] 		=	"SYS",
		};
		return __level_string[level];
	}

#ifdef HAS_LOG_LAYOUT
	struct EasylogLayoutNode {
		std::string plainstring;
		int arg = -1;
		std::function<void(EasylogLayoutNode*, EasylogMessage*, std::ostream&)> dynamicstring = nullptr;
		EasylogLayoutNode(const char* s) : plainstring(s) {}
		EasylogLayoutNode(const char* s, size_t n) : plainstring(s, n) {}
		EasylogLayoutNode(std::string s) : plainstring(s) {}
		EasylogLayoutNode(EasylogLayoutNode* layoutNode) {
			this->plainstring = layoutNode->plainstring;
			this->arg = layoutNode->arg;
			this->dynamicstring = layoutNode->dynamicstring;
		}
		EasylogLayoutNode(std::function<void(EasylogLayoutNode*, EasylogMessage*, std::ostream&)> func) : dynamicstring(func) {}
	};
#endif

	struct EasylogLevelNode {
		EasylogSeverityLevel level;
		EasylogColor color;
		bool to_stdout;
		std::string filename;
		std::ofstream* fs;
		u64 fs_launchtime;
#ifdef HAS_LOG_LAYOUT				
		std::list<EasylogLayoutNode*> layouts_prefix;
		std::list<EasylogLayoutNode*> layouts_postfix;
#endif				
	};

	struct EasylogNode {
		EasylogLevelNode* levelNode;
		std::stringbuf buffer;
		EasylogNode* next;
		EasylogNode() : levelNode(nullptr), next(nullptr) {}
	};


	EasylogMessage::EasylogMessage(Easylog* easylog, EasylogSeverityLevel level, std::string file, int line, std::string func)
		: std::ostream(nullptr)
		, _easylog(easylog)
		, _level(level)
#ifdef HAS_LOG_LAYOUT		
		, _file(file)
		, _line(line)
		, _function(func)
#endif
		, _log(new EasylogNode())
	{
		rdbuf(&this->_log->buffer);

#ifdef HAS_LOG_LAYOUT
		const std::list<EasylogLayoutNode*>& layouts = this->_easylog->layout_prefix(this->level());
		for (auto& layoutNode : layouts) {
			if (layoutNode->dynamicstring != nullptr) {
				layoutNode->dynamicstring(layoutNode, this, *this);
			}
			else {
				*this << layoutNode->plainstring;
			}
		}
#else

		sTime.now();

#ifdef ENABLE_PREFIX_DATETIME

		char time_buffer[64];
		
#ifdef ENABLE_PREFIX_DATETIME_LONG
		timestamp(time_buffer, sizeof(time_buffer), sTime.secondPart(), "[%y/%02m/%02d %02H:%02M:%02S");
#else
		timestamp(time_buffer, sizeof(time_buffer), sTime.secondPart(), "[%02H:%02M:%02S");
			//easylog->autosplit_hour() ? "[%02M:%02S" : "[%02H:%02M:%02S");
#endif

		*this << time_buffer;

#ifdef ENABLE_PREFIX_DATETIME_MILLISECOND
		*this << "|" << std::setw(3) << std::setfill('0') << sTime.millisecondPart() << "] ";
#else
		*this << "] ";
#endif

#endif

#ifdef ENABLE_PREFIX_LEVEL
		if (level != LEVEL_TRACE) {
			*this << "<" << levelshort_string(level) << "> ";
		}
#endif

#ifdef ENABLE_PREFIX_ERROR_FILE
		if (level == LEVEL_ERROR || level == LEVEL_PANIC) {
			*this << "(" << file << ":" << line << ") ";
		}
#endif

#endif
	}

	EasylogMessage::~EasylogMessage() {
#ifdef HAS_LOG_LAYOUT
		const std::list<EasylogLayoutNode*>& layouts = this->_easylog->layout_postfix(this->level());
		for (auto& layoutNode : layouts) {
			if (layoutNode->dynamicstring != nullptr) {
				layoutNode->dynamicstring(layoutNode, this, *this);
			}
			else {
				*this << layoutNode->plainstring;
			}
		}
#endif	
		this->flush();
	}

	void EasylogMessage::flush() {
		if (this->tellp() > 0) {
			*this << "\n";
			this->_easylog->log_message(this);
		}
	}

	void EasylogMessage::cout(const char* format, ...) {
		char log[65536];
		va_list va;
		va_start(va, format);
		int len = vsnprintf(log, sizeof(log), format, va);
		va_end(va);
		if (len > 0) {
			*this << log;
		}
		else {
			fprintf(stderr, "cout error:%d,%s", errno, strerror(errno));
		}
	}

	////////////////////////////////////////////////////////////////////////////////////

	class EasylogInternal : public Easylog {
		public:
			EasylogInternal();
			~EasylogInternal();

		public:
			EasylogSeverityLevel level() override { return this->_level; }
			void set_level(EasylogSeverityLevel level) override { this->_level = level; }
			EasylogColor color(EasylogSeverityLevel level) override { return this->_levels[level].color; }
			void set_color(EasylogSeverityLevel level, EasylogColor color) override;
			bool set_destination(std::string dir) override;
			void set_tostdout(EasylogSeverityLevel level, bool enable) override;
			void set_toserver(EasylogSeverityLevel level, std::string address, int port) override;
			void set_tofile(EasylogSeverityLevel level, std::string filename) override;
			bool autosplit_day() override { return this->_autosplit_day; }
			bool autosplit_hour() override { return this->_autosplit_hour; }
			void set_autosplit_day(bool value) override { this->_autosplit_day = value; }
			void set_autosplit_hour(bool value) override { this->_autosplit_hour = value; }
#ifdef HAS_LOG_LAYOUT			
			bool set_layout(EasylogSeverityLevel level, std::string layout) override;
			const std::list<EasylogLayoutNode*>& layout_prefix(EasylogSeverityLevel level) override { return this->_levels[level].layouts_prefix; }
			const std::list<EasylogLayoutNode*>& layout_postfix(EasylogSeverityLevel level) override { return this->_levels[level].layouts_postfix; }
#endif
			inline bool isstop() { return this->_stop; }
			void stop() override;

		public:
			void log_message(EasylogMessage* easylogMessage) override;

		private:
			bool _stop = false;
#if defined(DEBUG) || defined(_DEBUG)
			EasylogSeverityLevel _level = LEVEL_DEBUG;
#else
			EasylogSeverityLevel _level = LEVEL_TRACE;
#endif
			std::string _dest_dir = get_current_dir_name();
			bool _autosplit_day = true, _autosplit_hour = false;
			void full_filename(const std::string& filename, std::string& fullname);

		private:

			void openfile(EasylogLevelNode* levelNode);
			void autosplit_file(EasylogLevelNode* levelNode);
			
			void send_to_stdout(EasylogLevelNode* levelNode, const std::string& s);
			void send_to_file(EasylogLevelNode* levelNode, const std::string& s);
			void send_to_network(EasylogLevelNode* levelNode, const std::string& s);

			// unordered_map MUST gcc version is above 7
			//std::unordered_map<EasylogSeverityLevel, EasylogLevelNode> _levels = {
			std::map<EasylogSeverityLevel, EasylogLevelNode> _levels = {
				{ LEVEL_DEBUG, 
{ level:LEVEL_DEBUG, color:LBLUE, to_stdout:false, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},

				{ LEVEL_TRACE, 
{ level:LEVEL_TRACE, color:GREY, to_stdout:false, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_ALARM, 
{ level:LEVEL_ALARM, color:YELLOW, to_stdout:true, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_ERROR, 
{ level:LEVEL_ERROR, color:LRED, to_stdout:true, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_PANIC, 
{ level:LEVEL_PANIC, color:LMAGENTA, to_stdout:true, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
				{ LEVEL_SYSTEM, 
{ level:LEVEL_SYSTEM, color:LCYAN, to_stdout:true, filename:"", fs:nullptr, fs_launchtime:0
#ifdef HAS_LOG_LAYOUT
, layouts_prefix:{}, layouts_postfix:{} 
#endif
}},
			};

		private:
#ifdef ENABLE_ASYNC_SEND		
			LockfreeQueue<EasylogNode*> _logQueue;
			std::mutex _logMutex;
			std::condition_variable _logCondition;
			std::thread* _logthread = nullptr;
			void logProcess();
#endif			

#ifdef HAS_LOG_LAYOUT
		private:
			std::unordered_map<std::string, EasylogLayoutNode*> _initnodes = {
				{ "process", new EasylogLayoutNode(std::to_string(getpid())) },
				{ "thread", new EasylogLayoutNode(std::to_string((threadid()))) },
				{ "level", new EasylogLayoutNode([this](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << level_string(easylogMessage->level());
				}) },
				{ "levelshort", new EasylogLayoutNode([this](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << levelshort_string(easylogMessage->level());
				}) },
				{ "user", new EasylogLayoutNode(getlogin()) },
				{ "host", new EasylogLayoutNode(gethostname()) },
				{ "file", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->file();
				}) },
				{ "line", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->line();
				}) },
				{ "function", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << easylogMessage->function();
				}) },
				{ "msg", nullptr},
				{ "datetime", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					char time_buffer[64];
					timestamp(time_buffer, sizeof(time_buffer), sTime.secondPart(), layoutNode->plainstring.c_str());
					os << time_buffer;
				}) },
				{ "millisecond", new EasylogLayoutNode([](EasylogLayoutNode* layoutNode, EasylogMessage* easylogMessage, std::ostream& os) {
					os << std::setw(layoutNode->arg) << std::setfill('0') << sTime.millisecondPart();
				}) }
			};
#endif

	};

	void EasylogInternal::send_to_stdout(EasylogLevelNode* levelNode, const std::string& s) {
		if (levelNode->to_stdout) {
			if (levelNode->color != GREY) {
				std::cout << "\x1b[" 
						<< (levelNode->color >= LRED ? (levelNode->color - 10) : levelNode->color) 
						<< (levelNode->color >= LRED ? ";1" : "") << "m";
				std::cout.write(s.data(), s.length() - 1);
				std::cout << "\x1b[0m" << std::endl;
			}
			else {
				std::cout.write(s.data(), s.length());
			}
		}
	}
	
	void EasylogInternal::send_to_file(EasylogLevelNode* levelNode, const std::string& s) {
		if (levelNode->fs != nullptr) {
			this->autosplit_file(levelNode);
			levelNode->fs->write(s.data(), s.length());
			levelNode->fs->flush();
		}	
	}
	
	void EasylogInternal::send_to_network(EasylogLevelNode* levelNode, const std::string& s) {
	}

	void EasylogInternal::log_message(EasylogMessage* easylogMessage) {
		if (!this->isstop() && easylogMessage->level() >= this->level()) {
			EasylogLevelNode* levelNode = &this->_levels[easylogMessage->level()];
			
#ifdef ENABLE_ASYNC_SEND
			easylogMessage->log()->levelNode = levelNode;
			this->_logQueue.push_back(easylogMessage->log());
			this->_logCondition.notify_all();
#else
			const std::string& s = easylogMessage->log()->buffer.str();
			this->send_to_stdout(levelNode, s);
			this->send_to_file(levelNode, s);
			this->send_to_network(levelNode, s);
			delete easylogMessage->log();
#endif
			if (easylogMessage->level() == LEVEL_PANIC) {
				this->stop();
				::abort();
			}
		}
		else {
#if 0			
			fprintf(stderr, "isstop: %s, easylogMessage->level: %d, this->level: %d, log: %s\n",
					this->isstop() ? "true" : "false",
					easylogMessage->level(),
					this->level(),
					easylogMessage->log()->buffer.str().c_str());
#endif			
		}
	}

#ifdef ENABLE_ASYNC_SEND
	void EasylogInternal::logProcess() {
		while (true) {
			EasylogNode* logNode = nullptr;
			if (!this->_logQueue.empty()) {
				logNode = this->_logQueue.pop_front();
			}
			if (logNode) {
				const std::string& s = logNode->buffer.str();
				this->send_to_stdout(logNode->levelNode, s);
				this->send_to_file(logNode->levelNode, s);
				this->send_to_network(logNode->levelNode, s);
				SafeDelete(logNode);
			}
			else {
				if (this->isstop()) { break; }
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
				std::unique_lock<std::mutex> locker(this->_logMutex);
				this->_logCondition.wait(locker);
				fprintf(stderr, "logProcess condition wakeup\n");
			}
		}
		fprintf(stderr, "Easylog exit, logQueue: %ld\n", this->_logQueue.size());
	}
#endif

#ifdef HAS_LOG_LAYOUT
	bool EasylogInternal::set_layout(EasylogSeverityLevel level, std::string layout) {
		auto parsefunc = [this](std::list<EasylogLayoutNode*>& layouts_prefix, std::list<EasylogLayoutNode*>& layouts_postfix, const std::string& layout) -> bool {
			std::string::size_type i = 0;
			bool msgNode = false;
			while (i < layout.length()) {
				std::string::size_type head = layout.find('{', i);
				if (head == std::string::npos) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
					return true;
				}

				std::string::size_type tail = layout.find('}', head);
				if (tail == std::string::npos) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
					return true;
				}

				if (head != i) {
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(std::string(layout, i, head - i));
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
				}

				std::string token, arg;				
				std::string::size_type colon = layout.find(':', head);
				if (colon == std::string::npos || colon > tail) {
					token.assign(layout, head + 1, tail - head - 1);
					arg.clear();
				}
				else {
					token.assign(layout, head + 1, colon - head - 1);
					arg.assign(layout, colon + 1, tail - colon - 1);
				}

				if (this->_initnodes.find(token) == this->_initnodes.end()) {
					fprintf(stderr, "illegal token: %s", token.c_str());
					return false;
				}

				if (this->_initnodes[token] == nullptr) {
					CHECK_RETURN(msgNode == false, false, "there is only unique {msg}");
					msgNode = true;
				}
				else {				
					EasylogLayoutNode* layoutNode = new EasylogLayoutNode(this->_initnodes[token]);
					if (!arg.empty()) {
						layoutNode->plainstring = arg;
						if (isdigit(layoutNode->plainstring)) {
							layoutNode->arg = atoi(layoutNode->plainstring.c_str());
						}
					}
					if (!msgNode) { layouts_prefix.push_back(layoutNode); } else { layouts_postfix.push_back(layoutNode); }
				}

				i = tail + 1;
			}
			
			return true;
		};

		auto clearfunc = [](std::list<EasylogLayoutNode*>& layouts) {
			for (auto& layoutNode : layouts) {
				SafeDelete(layoutNode);
			}
			layouts.clear();
		};

		bool result = false;
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				clearfunc(levelNode.layouts_prefix);
				clearfunc(levelNode.layouts_postfix);
				result = parsefunc(levelNode.layouts_prefix, levelNode.layouts_postfix, layout);
				if (!result) {
					clearfunc(levelNode.layouts_prefix);
					clearfunc(levelNode.layouts_postfix);
					break;
				}
			}
		}
		
		return result;
	}
#endif

	bool EasylogInternal::set_destination(std::string dir) {
		if (!existDir(dir.c_str()) && !createDirectory(dir.c_str())) {
			return false;
		}
		CHECK_RETURN(existDir(dir.c_str()), false, "dir: `%s` not existence", dir.c_str());
		CHECK_RETURN(isDir(dir.c_str()), false, "`%s` not directory", dir.c_str());
		CHECK_RETURN(accessableDir(dir.c_str()), false, "dir: `%s` not accessible", dir.c_str());
		CHECK_RETURN(writableDir(dir.c_str()), false, "dir: `%s` not writable", dir.c_str());
		this->_dest_dir = dir;
		return true;
	}

	void EasylogInternal::full_filename(const std::string& filename, std::string& fullname) {
		fullname = this->_dest_dir + "/" + filename;
		if (this->_autosplit_hour) {
			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer), 0, ".%Y-%02m-%02d.%02H");
			fullname += time_buffer;
		}
		else if (this->_autosplit_day) {
			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer), 0, ".%Y-%02m-%02d");
			fullname += time_buffer;
		}
	}
	
	void EasylogInternal::openfile(EasylogLevelNode* levelNode) {
		if (levelNode->fs != nullptr) {
			levelNode->fs->close();
			SafeDelete(levelNode->fs);
		}		
		std::string fullname;
		this->full_filename(levelNode->filename, fullname);
		try {
			levelNode->fs = new std::ofstream(fullname, std::ios::app|std::ios::out);
		} catch (std::exception& e) {
			fprintf(stderr, "ofstream exception: %s, filename: %s\n", e.what(), fullname.c_str());
			SafeDelete(levelNode->fs);
		}
		levelNode->fs_launchtime = timeSecond();	
	}

	void EasylogInternal::autosplit_file(EasylogLevelNode* levelNode) {
		if (this->_autosplit_day || this->_autosplit_hour) {
			u64 nowtime = sTime.secondPart();
			struct tm tm_nowtime, tm_launchtime;
			gmtime_r((const time_t *) &nowtime, &tm_nowtime);
			gmtime_r((const time_t *) &levelNode->fs_launchtime, &tm_launchtime);
			if ((this->_autosplit_day && tm_nowtime.tm_mday != tm_launchtime.tm_mday) 
				|| (this->_autosplit_hour && tm_nowtime.tm_hour != tm_launchtime.tm_hour)){
				this->openfile(levelNode);
			}
		}
	}

	void EasylogInternal::set_tostdout(EasylogSeverityLevel level, bool enable) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.to_stdout = enable;
			}
		}
	}
	
	void EasylogInternal::set_tofile(EasylogSeverityLevel level, std::string filename) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.filename = filename;
			}
		}

		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.filename.length() > 0 && levelNode.fs == nullptr) {
				this->openfile(&levelNode);
			}
		}
	}

	void EasylogInternal::set_toserver(EasylogSeverityLevel level, std::string address, int port) {
		//TODO: toserver by MessageQueue
	}
			
	void EasylogInternal::set_color(EasylogSeverityLevel level, EasylogColor color) {
		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.level == level || level == GLOBAL) {
				levelNode.color = color;
			}
		}
	}

	void EasylogInternal::stop() {
		this->_stop = true;
#ifdef ENABLE_ASYNC_SEND		
		this->_logCondition.notify_all();
		if (this->_logthread && this->_logthread->joinable()) {
			this->_logthread->join();
		}
		SafeDelete(this->_logthread);
#endif
	}
		
	
	EasylogInternal::EasylogInternal() {
#ifdef ENABLE_ASYNC_SEND	
		SafeDelete(this->_logthread);
		this->_logthread = new std::thread([this]() {
			this->logProcess();
		});
#endif		
	}

	Easylog::~Easylog() {}
	EasylogInternal::~EasylogInternal() {
#ifdef HAS_LOG_LAYOUT
		for (auto& i : this->_initnodes) {
			SafeDelete(i.second);
		}
		this->_initnodes.clear();
#endif		

		for (auto& i : this->_levels) {
			EasylogLevelNode& levelNode = i.second;
			if (levelNode.fs != nullptr) {
				levelNode.fs->close();
				SafeDelete(levelNode.fs);
			}

#ifdef HAS_LOG_LAYOUT
			for (auto& layoutNode : levelNode.layouts_prefix) {
				SafeDelete(layoutNode);
			}
			levelNode.layouts_prefix.clear();
			
			for (auto& layoutNode : levelNode.layouts_postfix) {
				SafeDelete(layoutNode);
			}
			levelNode.layouts_postfix.clear();
#endif			
		}
	}

	Easylog* EasylogCreator::create() {
		return new EasylogInternal();
	}

	Easylog* Easylog::syslog() {
		static Easylog* __syslog = EasylogCreator::create();
		return __syslog;
	}
}

