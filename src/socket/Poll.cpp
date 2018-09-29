/*
 * \file: Poll.cpp
 * \brief: Created by hushouguo at 14:39:56 Jun 28 2018
 */

#include "bundle.h"
#include "Poll.h"
#include "WorkerProcess.h"

BEGIN_NAMESPACE_BUNDLE {
	Poll::Poll(WorkerProcess* slotWorker) {
		this->_epfd = epoll_create(NM_POLL_EVENT); /* `NM_POLL_EVENT` is just a hint for the kernel */
		this->_slotWorker = slotWorker;
		memset(this->_events, 0, sizeof(this->_events));
	}
		
	Poll::~Poll() {
		::close(this->_epfd);
	}

	bool Poll::addSocket(SOCKET s) {
		struct epoll_event ee;
		ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, s, &ee);		
		CHECK_RETURN(rc == 0, false, "epoll_add error: %d, %s", errno, strerror(errno));
		return true;
	}

	bool Poll::removeSocket(SOCKET s) {
		struct epoll_event ee;
		ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_DEL, s, &ee);
		CHECK_RETURN(rc == 0, false, "epoll_del error: %d, %s", errno, strerror(errno));
		return true;
	}

	bool Poll::setSocketPollout(SOCKET s, bool value) {
		struct epoll_event ee;
		ee.events = value ? (EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR) : (EPOLLET | EPOLLIN | EPOLLERR);
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		int rc = epoll_ctl(this->_epfd, EPOLL_CTL_MOD, s, &ee);
		CHECK_RETURN(rc == 0, false, "epoll_mod error: %d, %s, socket: %d", errno, strerror(errno), s);
		return true;
	}

	void Poll::run(int milliseconds) {
		/* -1 to block indefinitely, 0 to return immediately, even if no events are available. */
		int numevents = ::epoll_wait(this->_epfd, this->_events, NM_POLL_EVENT, milliseconds);
		if (numevents < 0) {
			if (errno == EINTR) {
				return; // wake up by signal
			}
			CHECK_RETURN(false, void(0), "epoll wait error:%d, %s", errno, strerror(errno));
		}
		for (int i = 0; i < numevents; ++i) {
			struct epoll_event* ee = &this->_events[i];
			if (ee->events & (EPOLLERR | EPOLLHUP)) {
				Error.cout("fd: %d poll error or hup: %d", ee->data.fd, ee->events);
				this->_slotWorker->errorSocket(ee->data.fd);
			}
			else if (ee->events & EPOLLRDHUP) {
				Error.cout("fd: %d poll error or rdhup: %d", ee->data.fd, ee->events);
				this->_slotWorker->errorSocket(ee->data.fd);
			}
			else {
				if (ee->events & EPOLLIN) {
					this->_slotWorker->readSocket(ee->data.fd);
				}

				if (ee->events & EPOLLOUT) {
					this->_slotWorker->writeSocket(ee->data.fd);
				}
			}
		}
	}
}

