/*
 * \file: test_tools.cpp
 * \brief: Created by hushouguo at 16:17:48 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;
void test_tools() {
	auto performance_time = [](int times) {
		for (int i = 0; i < times; ++i) {
			std::time(nullptr);
		}
	};

	auto performance_gettimeofday = [](int times) {
		struct timeval tv;
		for (int i = 0; i < times; ++i) {
			gettimeofday(&tv, nullptr);
		}
	};

	auto performance_cpp11 = [](int times) {
		for (int i = 0; i < times; ++i) {
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	};

	if (true) {
		Time t1;
		performance_time(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times std::time cost milliseconds: " << t2 - t1;
	}

	if (true) {
		Time t1;
		performance_gettimeofday(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times gettimeofday cost milliseconds: " << t2 - t1;
	}
	
	if (true) {
		Time t1;
		performance_cpp11(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times c++11 chrono cost milliseconds: " << t2 - t1;
	}

	time_t nowtime = std::time(nullptr);

	auto performance_localtime = [nowtime](int times) {
		for (int i = 0; i < times; ++i) {
			localtime(&nowtime);	
		}
	};
	
	auto performance_localtime_r = [nowtime](int times) {
		struct tm result;
		for (int i = 0; i < times; ++i) {
			localtime_r(&nowtime, &result);
		}
	};
	
	auto performance_gmtime = [nowtime](int times) {
		for (int i = 0; i < times; ++i) {
			gmtime(&nowtime);	
		}
	};
	
	auto performance_gmtime_r = [nowtime](int times) {
		struct tm result;
		for (int i = 0; i < times; ++i) {
			gmtime_r(&nowtime, &result);
		}
	};
	
	auto performance_timestamp = [](int times) {
		char buffer[64];
		for (int i = 0; i < times; ++i) {
			timestamp(buffer, sizeof(buffer), 0, nullptr);
		}
	};

	auto performance_hashstring = [](int times) {
		std::string s = "For the average case, it is not always a big difference between the two. ";
		for (int i = 0; i < times; ++i) {
			hashString(s.data(), s.length());
		}
	};

	if (true) {
		Time t1;
		performance_localtime(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times localtime cost milliseconds: " << t2 - t1;
	}
	if (true) {
		Time t1;
		performance_localtime_r(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times localtime_r cost milliseconds: " << t2 - t1;
	}
	if (true) {
		Time t1;
		performance_gmtime(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times gmtime cost milliseconds: " << t2 - t1;
	}
	if (true) {
		Time t1;
		performance_gmtime_r(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times gmtime_r cost milliseconds: " << t2 - t1;
	}
	if (true) {
		Time t1;
		performance_timestamp(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times timestamp cost milliseconds: " << t2 - t1;
	}
	if (true) {
		Time t1;
		performance_hashstring(1000000);
		Time t2;
		Trace << "performance: run 1,000,000 times hashstring cost milliseconds: " << t2 - t1;
	}


	//base64
	if (true) {
		std::string buffer = "hello, world!";
		std::string encodestring;
		base64_encode((unsigned char*)buffer.data(), buffer.length(), encodestring);
		Trace << "base64 encode: " << encodestring;

		std::string rawstring;
		base64_decode(encodestring, rawstring);
		Trace << "base64 decode: " << rawstring;

		auto performance_base64_encode = [&encodestring](int times) {
			std::string buffer = "hello, world!";
			for (int i = 0; i < times; ++i) {
				base64_encode((unsigned char*)buffer.data(), buffer.length(), encodestring);
			}
		};

		auto performance_base64_decode = [&encodestring](int times) {
			std::string rawstring;
			for (int i = 0; i < times; ++i) {
				base64_decode(encodestring, rawstring);
			}
		};

		if (true) {
			Time t1;
			performance_base64_encode(1000000);
			Time t2;
			Trace << "performance: run 1,000,000 times base64_encode cost milliseconds: " << t2 - t1;
		}
		if (true) {
			Time t1;
			performance_base64_decode(1000000);
			Time t2;
			Trace << "performance: run 1,000,000 times base64_decode cost milliseconds: " << t2 - t1;
		}		
	}

	//TODO: ByteBuffer etc...
	//

	System << "test tools OK";
}
