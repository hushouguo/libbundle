/*
 * \file: Poll.cpp
 * \brief: Created by hushouguo at 14:39:56 Jun 28 2018
 */

#include "bundle.h"
#include "Poll.h"

BEGIN_NAMESPACE_BUNDLE {
	Poll::Poll() {
		this->_epfd = epoll_create(NM_POLL_EVENT); /* `NM_POLL_EVENT` is just a hint for the kernel */
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
}

