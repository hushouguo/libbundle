/*
 * \file: Poll.h
 * \brief: Created by hushouguo at 14:39:25 Jun 28 2018
 */
 
#ifndef __POLL_H__
#define __POLL_H__

#define NM_POLL_EVENT		128

BEGIN_NAMESPACE_BUNDLE {
	class Poll {
		public:
			Poll();
			~Poll();

		public:
			template < typename ReadHandler, typename WriteHandler, typename ErrorHandler >
			void run(int milliseconds, ReadHandler readHandler, WriteHandler writeHandler, ErrorHandler errorHandler) {
				/* -1 to block indefinitely, 0 to return immediately, even if no events are available. */
				//int numevents = TEMP_FAILURE_RETRY(::epoll_wait(this->_epfd, this->_events, NM_POLL_EVENT, milliseconds));
				int numevents = ::epoll_wait(this->_epfd, this->_events, NM_POLL_EVENT, milliseconds);
				CHECK_RETURN(numevents >= 0, void(0), "epoll wait error:%d, %s", errno, strerror(errno));
				for (int i = 0; i < numevents; ++i) {
					struct epoll_event* ee = &this->_events[i];
					if (ee->events & (EPOLLERR | EPOLLHUP)) {
						Error.cout("fd: %d poll error or hup: %d", ee->data.fd, ee->events);
						errorHandler(ee->data.fd);
					}
					else if (ee->events & EPOLLRDHUP) {
						Error.cout("fd: %d poll error or rdhup: %d", ee->data.fd, ee->events);
						errorHandler(ee->data.fd);
					}
					else {
						if (ee->events & EPOLLIN) {
							readHandler(ee->data.fd);
						}
						
						if (ee->events & EPOLLOUT) {
							writeHandler(ee->data.fd);
						}
					}
				}			
			}
			
		public:
			bool addSocket(SOCKET s);
			bool removeSocket(SOCKET s);
			bool setSocketPollout(SOCKET s, bool value);

		private:
			int _epfd = -1;
			struct epoll_event _events[NM_POLL_EVENT];
	};
}

#endif
