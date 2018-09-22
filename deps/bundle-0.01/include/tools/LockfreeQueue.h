/*
 * \file: LockfreeQueue.h
 * \brief: Created by hushouguo at 20:02:26 Sep 20 2018
 */
 
#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

BEGIN_NAMESPACE_BUNDLE {
	template <typename T> class LockfreeQueue {
		public:
			inline void push_back(T node);
			inline void push_front(T node);
			inline T pop_back();
			inline T pop_front();
			inline size_t size();
			inline bool empty();

		private:
			Spinlocker _locker;
			std::list<T> _list;
	};

	template <typename T> void LockfreeQueue<T>::push_back(T node) {
		this->_locker.lock();
		this->_list.push_back(node);
		this->_locker.unlock();
	}
	
	template <typename T> void LockfreeQueue<T>::push_front(T node) {
		this->_locker.lock();
		this->_list.push_front(node);
		this->_locker.unlock();
	}

	template <typename T> T LockfreeQueue<T>::pop_back() {
		assert(this->empty() == false);
		this->_locker.lock();
		T node = this->_list.back();
		this->_list.pop_back();
		this->_locker.unlock();
		return node;
	}
	
	template <typename T> T LockfreeQueue<T>::pop_front() {
		assert(this->empty() == false);
		this->_locker.lock();
		T node = this->_list.front();
		this->_list.pop_front();
		this->_locker.unlock();
		return node;
	}
	
	template <typename T> size_t LockfreeQueue<T>::size() {
		return this->_list.size();
	}
	
	template <typename T> bool LockfreeQueue<T>::empty() {
		return this->_list.empty();
	}
}

#endif
