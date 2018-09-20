/*
 * \file: test_easylog.cpp
 * \brief: Created by hushouguo at 15:55:41 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;
void test_easylog() {
	u8  val_u8 = 65;
	s8  val_s8 = '*';
	u16 val_u16 = 100;
	s16 val_s16 = -100;
	u32 val_u32 = 1000;
	s32 val_s32 = -1000;
	u64 val_u64 = 1234567890000;
	s64 val_s64 = -1234567890000;
	char val_char = 'a';
	bool val_bool = true;
	float val_float = 1.23456;
	double val_double = -2.345351;
	const char* val_cstr = "i am a const string";
	std::string val_string = "i am a string";

	Easylog::syslog()->set_level(GLOBAL);
	Easylog::syslog()->set_tostdout(GLOBAL, true);

	Trace << "u8: " << val_u8;
	Trace << "s8: " << val_s8;
	Trace << "u16: " << val_u16;
	Trace << "s16: " << val_s16;
	Trace << "u32: " << val_u32;
	Trace << "s32: " << val_s32;
	Trace << "u64: " << val_u64;
	Trace << "s64: " << val_s64;
	Trace << "char: " << val_char;
	Trace << "bool: " << val_bool;
	Trace << "float: " << val_float;
	Trace << "double: " << val_double;
	Trace << "cstr: " << val_cstr;
	Trace << "string: " << val_string;

	Alarm << "i am a alarm log";
	Error << "i am a error log";
	Panic << "i am a panic log";

	sleep(2);
	
	Easylog::syslog()->set_autosplit_day(true);
	Easylog::syslog()->set_destination(".logs");
	Easylog::syslog()->set_tofile(GLOBAL, "tests");
	Easylog::syslog()->set_tostdout(GLOBAL, false);

	auto performance_single_thread = [](int times) {
		for (int i = 0; i < times; ++i) {
			Trace << "For the average case, it is not always a big difference between the two. " << i;
		}
	};

	auto performance_multi_thread = [](int threads, int times) {
		std::vector<std::thread*> v;
		for (int i = 0; i < threads; ++i) {
			v.push_back(new std::thread([i, times](){
				int j = 0;
				for (; j < times; ++j) {
					Trace << "[" << i << "] For the average case, it is not always a big difference between the two. " << j;
				}
			}));
		}
		for (auto& t : v) {
			if (t->joinable()) {
				t->join();
			}
		}
	};

	if (true) {
		Time t1;
		performance_single_thread(1000000);
		Time t2;
		fprintf(stderr, "performance: single thread cost milliseconds: %ld, times: 1,000,000\n", t2 - t1);
	}

	if (true) {
		Time t1;
		performance_multi_thread(8, 500000);
		Time t2;
		fprintf(stderr, "performance: 8 threads cost milliseconds: %ld, times: 500,000\n", t2 - t1);
	}

	//Easylog::syslog()->stop();

	System << "easylog test OK";
}
