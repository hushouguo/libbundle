/*
 * \file: test_lockfree.cpp
 * \brief: Created by hushouguo at 00:29:44 Sep 21 2018
 */

#include "bundle.h"

using namespace bundle;

struct Node {
	int id;
	Node* next;
	Node(int value) : id(value), next(nullptr) {}
};

LockfreeQueue<Node> Q;

void test_lockfree() {
	auto producter = [](u32 size) {
		for (u32 i = 0; i < size; ++i) {
			Q.push_back(new Node(i));
		}
	};	

	auto consumer = [](int threadid) {
		u32 count = 0;
		while (true) {
			Node* node = Q.pop_front();
			if (node) {
				++count;
				delete node;
			}
			else {
				break;
			}
		};
		fprintf(stderr, "thread: %d, count: %u\n", threadid, count);
	};

	std::vector<std::thread*> v;

	for (int n = 0; n < 4; ++n) {
		v.push_back(new std::thread(producter, 1000000));
	}

	Time t1;

	for (int n = 0; n < 4; ++n) {
		v.push_back(new std::thread(consumer, n));
	}

	for (auto& i : v) {
		i->join();
	}

	Time t2;

	fprintf(stderr, "cost milliseconds: %ld, size: %ld\n", t2 - t1, Q.size());
}
