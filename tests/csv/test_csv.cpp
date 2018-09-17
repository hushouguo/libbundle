/*
 * \file: test_csv.cpp
 * \brief: Created by hushouguo at 18:58:51 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;

void test_csv(const char* filename) {
	parseCsv<int, int, int, int>(filename, "time", "dir", "lines", "delay", 
		[](int row, int& time, int& dir, int& lines, int& delay) {
			Trace << "row: " << row << ", time: " << time << ", dir: " << dir << ", lines: " << lines << ", delay: " << delay;
		});
	System << "test csv OK";
}
