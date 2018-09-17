/*
 * \file: CsvParser.h
 * \brief: Created by hushouguo at 16:32:00 Aug 26 2018
 */
 
#ifndef __CSV_PARSER_H__
#define __CSV_PARSER_H__

BEGIN_NAMESPACE_BUNDLE {
	// 1
	template <
			typename T1 
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					std::function<void(int row, T1&)> func) { 
			try {
				io::CSVReader<1> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1);
				int row = 0;
				T1 v1;
				while(in.read_row(v1)) {
					func(row++, v1);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 2	
	template <
			typename T1, 
			typename T2 
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					std::function<void(int row, T1&, T2&)> func) { 
			try {
				io::CSVReader<2> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2);
				int row = 0;
				T1 v1;
				T2 v2;
				while(in.read_row(v1, v2)) {
					func(row++, v1, v2);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	
	// 3	
	template <
			typename T1, 
			typename T2, 
			typename T3 
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					std::function<void(int row, T1&, T2&, T3&)> func) { 
			try {
				io::CSVReader<3> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				while(in.read_row(v1, v2, v3)) {
					func(row++, v1, v2, v3);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 4
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4 
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					std::function<void(int row, T1&, T2&, T3&, T4&)> func) { 
			try {
				io::CSVReader<4> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				while(in.read_row(v1, v2, v3, v4)) {
					func(row++, v1, v2, v3, v4);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 5	
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5 
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&)> func) { 
			try {
				io::CSVReader<5> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				while(in.read_row(v1, v2, v3, v4, v5)) {
					func(row++, v1, v2, v3, v4, v5);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 6
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&)> func) { 
			try {
				io::CSVReader<6> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				while(in.read_row(v1, v2, v3, v4, v5, v6)) {
					func(row++, v1, v2, v3, v4, v5, v6);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 7
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&)> func) { 
			try {
				io::CSVReader<7> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 8
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&)> func) { 
			try {
				io::CSVReader<8> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 9
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&)> func) { 
			try {
				io::CSVReader<9> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 10
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&)> func) { 
			try {
				io::CSVReader<10> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 11
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&)> func) { 
			try {
				io::CSVReader<11> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 12
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&)> func) { 
			try {
				io::CSVReader<12> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 13
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&)> func) { 
			try {
				io::CSVReader<13> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 14
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&)> func) { 
			try {
				io::CSVReader<14> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 15
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&)> func) { 
			try {
				io::CSVReader<15> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 16
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15,
			typename T16
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					const char* k16,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&, T16&)> func) { 
			try {
				io::CSVReader<16> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15,k16);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				T16 v16;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 17
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15,
			typename T16,
			typename T17
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					const char* k16,
					const char* k17, 
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&, T16&, T17&)> func) { 
			try {
				io::CSVReader<17> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15,k16,k17);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				T16 v16;
				T17 v17;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 18
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15,
			typename T16,
			typename T17,
			typename T18
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					const char* k16,
					const char* k17, 
					const char* k18,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&, T16&, T17&, T18&)> func) { 
			try {
				io::CSVReader<18> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15,k16,k17,k18);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				T16 v16;
				T17 v17;
				T18 v18;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 19
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15,
			typename T16,
			typename T17,
			typename T18,
			typename T19
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					const char* k16,
					const char* k17, 
					const char* k18,
					const char* k19,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&, T16&, T17&, T18&, T19&)> func) { 
			try {
				io::CSVReader<19> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15,k16,k17,k18,k19);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				T16 v16;
				T17 v17;
				T18 v18;
				T19 v19;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}

	// 20
	template <
			typename T1, 
			typename T2, 
			typename T3, 
			typename T4, 
			typename T5, 
			typename T6, 
			typename T7,
			typename T8,
			typename T9,
			typename T10,
			typename T11,
			typename T12,
			typename T13,
			typename T14,
			typename T15,
			typename T16,
			typename T17,
			typename T18,
			typename T19,
			typename T20
			>
		bool parseCsv(std::string file, 
					const char* k1, 
					const char* k2,
					const char* k3,
					const char* k4,
					const char* k5, 
					const char* k6,
					const char* k7,
					const char* k8,
					const char* k9, 
					const char* k10,
					const char* k11,
					const char* k12,
					const char* k13, 
					const char* k14,
					const char* k15,
					const char* k16,
					const char* k17, 
					const char* k18,
					const char* k19,
					const char* k20,
					std::function<void(int row, T1&, T2&, T3&, T4&, T5&, T6&, T7&, T8&, T9&, T10&, T11&, T12&, T13&, T14&, T15&, T16&, T17&, T18&, T19&, T20&)> func) { 
			try {
				io::CSVReader<20> in(file);
				in.next_line();//ignore chinese comment
				in.read_header(io::ignore_extra_column, k1,k2,k3,k4,k5,k6,k7,k8,k9,k10,k11,k12,k13,k14,k15,k16,k17,k18,k19,k20);
				int row = 0;
				T1 v1;
				T2 v2;
				T3 v3;
				T4 v4;
				T5 v5;
				T6 v6;
				T7 v7;
				T8 v8;
				T9 v9;
				T10 v10;
				T11 v11;
				T12 v12;
				T13 v13;
				T14 v14;
				T15 v15;
				T16 v16;
				T17 v17;
				T18 v18;
				T19 v19;
				T20 v20;
				while(in.read_row(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20)) {
					func(row++, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20);
				}
			}	
			catch(std::exception& e) {
				CHECK_RETURN(false, false, "parseCsv: %s, exception: %s", file.c_str(), e.what());
			}
			return true;
		}
}

#endif
