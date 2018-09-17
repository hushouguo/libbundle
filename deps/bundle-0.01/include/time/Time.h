/*
 * \file: Time.h
 * \brief: Created by hushouguo at Jul 06 2017 20:46:33
 */
 
#ifndef __TIME_H__
#define __TIME_H__

BEGIN_NAMESPACE_BUNDLE {
	class Time {
		public:
			Time();
			Time(u64 milliseconds);
			Time(u64 second_part, u64 millisecond_part);
			Time(const Time& t);

		public:
			void now();
			inline u64 secondPart() const { return this->_tv.tv_sec; }
			inline u64 millisecondPart() const { return this->_tv.tv_usec / 1000; }
			inline u64 microsecondsPart() const { return this->_tv.tv_usec; }
			inline u64 milliseconds() const { return this->_tv.tv_sec * 1000 + this->millisecondPart(); }
			
		public:
			void operator =(u64 milliseconds);
			void operator =(const Time& t);
			u64  operator +(u64 milliseconds);
			u64  operator +(const Time& t);
			u64  operator -(u64 milliseconds);
			u64  operator -(const Time& t);
			bool operator >(const Time& t);
			bool operator <(const Time& t);			
			
		private:
			struct timeval _tv = {tv_sec:0, tv_usec:0};
	};
}

#define sTime bundle::Singleton<bundle::Time>::getInstance()

#endif
