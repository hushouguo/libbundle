/*
 * \file: Easylog.h
 * \brief: Created by hushouguo at 17:45:41 Aug 10 2018
 */
 
#ifndef __EASYLOG_H__
#define __EASYLOG_H__

BEGIN_NAMESPACE_BUNDLE {
	enum EasylogSeverityLevel {
		GLOBAL	=	0,
		TRACE	=	1,
		ALARM	=	2,
		ERROR	=	3,
		PANIC	=	4,
		SYSTEM	=	5,
		MAX_LEVEL =	6,
	};

#define Trace	bundle::EasylogMessage(bundle::Easylog::syslog(), bundle::TRACE, __FILE__, __LINE__, __FUNCTION__)
#define Alarm	bundle::EasylogMessage(bundle::Easylog::syslog(), bundle::ALARM, __FILE__, __LINE__, __FUNCTION__)
#define Error	bundle::EasylogMessage(bundle::Easylog::syslog(), bundle::ERROR, __FILE__, __LINE__, __FUNCTION__)
#define Panic	bundle::EasylogMessage(bundle::Easylog::syslog(), bundle::PANIC, __FILE__, __LINE__, __FUNCTION__)
#define System	bundle::EasylogMessage(bundle::Easylog::syslog(), bundle::SYSTEM, __FILE__, __LINE__, __FUNCTION__)

#ifdef assert
#undef assert
#endif
#define assert(condition)	\
		do {\
			if (!(condition)) {\
				Panic.cout("Assert: %s", #condition);\
				bundle::Easylog::syslog()->stop();\
				::abort();\
			}\
		} while(0)
	
#ifdef Assert
#undef Assert
#endif
#define Assert(condition, format, ...)	\
		do {\
			if (!(condition)) {\
				Panic.cout("Assert: %s, %s:%d\ncondition: %s, " format, __FILE__, __FUNCTION__, __LINE__, #condition, ##__VA_ARGS__);\
				bundle::Easylog::syslog()->stop();\
				::abort();\
			}\
		} while(0)
	
#define CHECK_RETURN(RC, RESULT, MESSAGE, ...)	\
		do {\
			if (!(RC)) {\
				Error.cout(MESSAGE, ##__VA_ARGS__);\
				return RESULT;\
			}\
		} while(false)
	
#define CHECK_GOTO(RC, SYMBOL, MESSAGE, ...)	\
		do {\
			if (!(RC)) {\
				Error.cout(MESSAGE, ##__VA_ARGS__);\
				goto SYMBOL;\
			}\
		} while(false)
	
#define CHECK_BREAK(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error.cout(MESSAGE, ##__VA_ARGS__);\
			break;\
		}
	
#define CHECK_CONTINUE(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error.cout(MESSAGE, ##__VA_ARGS__);\
			continue;\
		}
	
#define CHECK_TRACE(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Trace.cout(MESSAGE, ##__VA_ARGS__);\
		}
			
#define CHECK_ALARM(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Alarm.cout(MESSAGE, ##__VA_ARGS__);\
		}

#define CHECK_ERROR(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Error.cout(MESSAGE, ##__VA_ARGS__);\
		}

#define CHECK_PANIC(RC, MESSAGE, ...)	\
		if (!(RC)) {\
			Panic.cout(MESSAGE, ##__VA_ARGS__);\
		}

	
	enum EasylogColor { 
		BLACK	=	30,
		RED 	=	31,
		GREEN	=	32,
		BROWN	=	33,
		BLUE	=	34,
		MAGENTA =	35,
		CYAN	=	36,
		GREY	=	37,
		LRED	=	41,
		LGREEN	=	42,
		YELLOW	=	43,
		LBLUE	=	44,
		LMAGENTA=	45,
		LCYAN	=	46,
		WHITE	=	47
	};

	class Easylog;
	class EasylogNode;
	class EasylogMessage : public std::ostream {
		public:
			EasylogMessage(Easylog* easylog, EasylogSeverityLevel level, std::string file, int line, std::string func);
			~EasylogMessage();

		public:
			void cout(const char* format, ...);

		public:
			inline EasylogSeverityLevel level() { return this->_level; }
#ifdef HAS_LOG_LAYOUT			
			inline const std::string& file() { return this->_file; }
			inline int line() { return this->_line; }
			inline const std::string& function() { return this->_function; }
#endif			
			inline EasylogNode* log() { return this->_log; }
			
		private:
			Easylog* _easylog = nullptr;
			EasylogSeverityLevel _level;
#ifdef HAS_LOG_LAYOUT			
			std::string _file;
			int _line = -1;
			std::string _function;
#endif			
			EasylogNode* _log = nullptr;
			void flush();
	};

	class Easylog {
		public:
			virtual ~Easylog() = 0;

		public:
			virtual EasylogSeverityLevel level() = 0;
			virtual void set_level(EasylogSeverityLevel level) = 0;
			virtual EasylogColor color(EasylogSeverityLevel level) = 0;
			virtual void set_color(EasylogSeverityLevel level, EasylogColor color) = 0;
			virtual bool set_destination(std::string dir) = 0;
			virtual void set_tostdout(EasylogSeverityLevel level, bool enable) = 0;
			virtual void set_toserver(EasylogSeverityLevel level, std::string address, int port) = 0;
			virtual void set_tofile(EasylogSeverityLevel level, std::string filename) = 0;
			virtual bool autosplit_day() = 0;
			virtual void set_autosplit_day(bool value) = 0;
			virtual bool autosplit_hour() = 0;
			virtual void set_autosplit_hour(bool value) = 0;
			
#ifdef HAS_LOG_LAYOUT			
			virtual bool set_layout(EasylogSeverityLevel level, std::string layout) = 0;
			virtual const std::list<LayoutNode*>& layout_prefix(EasylogSeverityLevel level) = 0;
			virtual const std::list<LayoutNode*>& layout_postfix(EasylogSeverityLevel level) = 0;
#endif			
			virtual void stop() = 0;

		public:
			static Easylog* syslog();
			
		private:
			friend class EasylogMessage;
			virtual void log_message(EasylogMessage* easylogMessage) = 0;
	};

	struct EasylogCreator {
		static Easylog* create();
	};
}

#endif
