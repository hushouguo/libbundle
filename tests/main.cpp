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

int main() {
	struct sigaction act;
	act.sa_handler = [](int sig) {
		fprintf(stderr, "main thread receive signal: %d\n", sig);
	};
	sigemptyset(&act.sa_mask);  
	sigaddset(&act.sa_mask, SIGALRM);
	act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
	sigaction(SIGALRM, &act, nullptr);		

	bundle::Easylog::syslog()->set_tostdout(bundle::GLOBAL, true);

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
