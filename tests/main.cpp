/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 16:43:49 Sep 06 2018
 */

#include "bundle.h"
#include "easylog/test_easylog.h"
#include "tools/test_tools.h"
#include "net/test_net.h"
#include "csv/test_csv.h"
#include "xml/test_xml.h"
#include "recordclient/test_recordclient.h"
#include "entity/test_entity.h"
#include "lockfree/test_lockfree.h"

int epfd = epoll_create(128);
int main() {	
	bundle::Easylog::syslog()->set_tostdout(bundle::GLOBAL, true);

#if 0
	auto addSocket = [](int s, u32 events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR){
		fprintf(stderr, "addSocket: %d\n", s);
		struct epoll_event ee;
		//ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.events = events;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		epoll_ctl(epfd, EPOLL_CTL_ADD, s, &ee);
	};

	auto removeSocket = [](int s) {
		fprintf(stderr, "removeSocket: %d\n", s);
		struct epoll_event ee;
		ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		epoll_ctl(epfd, EPOLL_CTL_DEL, s, &ee);
	};

	auto modifySocket = [](int s, int flags) {
		fprintf(stderr, "modifySocket: %d\n", s);
		struct epoll_event ee;
		ee.events = flags;
		ee.data.u64 = 0; /* avoid valgrind warning */
		ee.data.fd = s;
		/* Note, Kernel < 2.6.9 requires a non null event pointer even for EPOLL_CTL_DEL. */
		epoll_ctl(epfd, EPOLL_CTL_MOD, s, &ee);
	};

	std::thread* epoll_thread = new std::thread([](){
				struct epoll_event events[128];
				while (1) {
					fprintf(stderr, "epoll_wait\n");
					int numevents = epoll_wait(epfd, events, 128, -1);
					fprintf(stderr, "epoll_wait wakeup: %d\n", numevents);
				}
			});

	sleep(1);
	int s = socket(AF_INET, SOCK_STREAM, 0);
	sleep(1);
	addSocket(s);
	sleep(1);
	removeSocket(s);
	sleep(1);
	addSocket(s);
	sleep(1);
	modifySocket(s, EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR);
	sleep(1);
	modifySocket(s, EPOLLET | EPOLLIN | EPOLLERR);
	sleep(1);
	modifySocket(s, EPOLLET | EPOLLIN | EPOLLOUT | EPOLLERR);
	sleep(1);
	addSocket(0, EPOLLET | EPOLLOUT | EPOLLERR);
	sleep(1);
	addSocket(0, EPOLLET | EPOLLOUT | EPOLLERR);
	sleep(1);

	epoll_thread->join();
#endif

#if 0
	struct sigaction act;
	act.sa_handler = [](int sig) {
		fprintf(stderr, "main thread receive signal: %d\n", sig);
	};
	sigemptyset(&act.sa_mask);  
	sigaddset(&act.sa_mask, SIGALRM);
	act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
	sigaction(SIGALRM, &act, nullptr);		
#endif

	//test_tools();
	test_net();
	//test_csv("./csv/test.csv");
	//test_xml("./xml/conf.xml");
	//test_xml("../server/recordserver/conf/db.xml");
	//test_xml2("../server/recordserver/conf/db.xml");
	//test_easylog();
	//test_recordclient("127.0.0.1", 9990);
	//test_entity();
	//test_lockfree();

	bundle::Easylog::syslog()->stop();
	return 0;
}
