/*
 * \file: Timer.h
 * \brief: Created by hushouguo at Jul 06 2017 20:46:48
 */
 
#ifndef __TIMER_H__
#define __TIMER_H__

BEGIN_NAMESPACE_BUNDLE {
	// base on milliseconds
	class Timer {
		public:
			Timer()	: _last_scale(sTime.milliseconds()), _interval_milliseconds(0) {
			}
			
			Timer(u32 milliseconds) : _last_scale(sTime.milliseconds()), _interval_milliseconds(milliseconds) {
				//assert(this->Interval() > 0);
			}

		public:
			inline void resetInterval(u32 milliseconds) { 
				this->_interval_milliseconds = milliseconds; 
			}
			
			inline u32 interval() { 
				return this->_interval_milliseconds; 
			}
			
			inline bool operator() () {
				if ((sTime.milliseconds() - this->_last_scale) >= this->_interval_milliseconds) {
					this->_last_scale = sTime.milliseconds();
					return true;
				}
				return false;
			}

			inline void resetScale() {
				this->_last_scale = sTime.milliseconds();
			}

		private:
			u64 _last_scale = 0;
			u32 _interval_milliseconds = 0;
	};	
}

#endif
