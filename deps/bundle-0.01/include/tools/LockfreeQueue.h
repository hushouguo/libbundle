/*
 * \file: LockfreeQueue.h
 * \brief: Created by hushouguo at 20:02:26 Sep 20 2018
 */
 
#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

BEGIN_NAMESPACE_BUNDLE {
	template <typename T> class LockfreeQueue {
		public:
			inline void push_back(T* node);
			inline T* pop_front();
			inline size_t size();

		private:
			Spinlocker _locker;
			T *_headNode = nullptr, *_tailNode = nullptr;
			size_t _size = 0;
	};

	template <typename T> void LockfreeQueue<T>::push_back(T* node) {
		node->next = nullptr;
		this->_locker.lock();
		if (this->_tailNode) {
			this->_tailNode->next = node;
			this->_tailNode = node;
		}
		else {
			assert(this->_headNode == nullptr);
			this->_headNode = this->_tailNode = node;
		}
		++this->_size;
		this->_locker.unlock();
	}

	template <typename T> T* LockfreeQueue<T>::pop_front() {
		T* node = nullptr;
		this->_locker.lock();
		if (this->_headNode) {
			node = this->_headNode;
			this->_headNode = this->_headNode->next;
			if (node == this->_tailNode) {
				assert(this->_headNode == nullptr);
				this->_tailNode = nullptr;
			}
			--this->_size;
		}
		this->_locker.unlock();
		return node;
	}
	
	template <typename T> size_t LockfreeQueue<T>::size() {
		return this->_size;
	}
}

#endif
