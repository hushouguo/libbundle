/*
 * \file: Spinlocker.h
 * \brief: Created by hushouguo at 23:34:20 Sep 20 2018
 */
 
#ifndef __SPINLOCKER_H__
#define __SPINLOCKER_H__

BEGIN_NAMESPACE_BUNDLE {
	class Spinlocker {
		public:
			void lock() {
				while (this->_locker.test_and_set(std::memory_order_acquire));
			}

			void unlock() {
				this->_locker.clear(std::memory_order_release);
			}
		private:
			std::atomic_flag _locker = ATOMIC_FLAG_INIT;
	};
}

#endif
