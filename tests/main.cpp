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

std::mutex mtx;
std::condition_variable cv;

bool isstop = false;
int main() {
	bundle::Easylog::syslog()->set_tostdout(bundle::GLOBAL, true);
	auto thread1 = [](int id) {
		fprintf(stderr, "thread:%d start\n", id);
		while (!isstop) {
			std::unique_lock<std::mutex> lk(mtx);
			cv.wait(lk);
			fprintf(stderr, "thread:%d wakeup\n", id);
		}
		fprintf(stderr, "thread:%d exit\n", id);
	};


	std::vector<std::thread*> v;
	for (int n = 0; n < 8; ++n) {
		v.push_back(new std::thread(thread1, n));
	}

	sleep(1);

	for (int n = 0; n < 10; ++n) {
		fprintf(stderr, "notify:%d\n", n);
		cv.notify_one();
		sleep(1);
	}

	isstop = true;
	cv.notify_all();

	for (auto& i : v) {
		i->join();
	}

	//test_tools();
	//test_net();
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
