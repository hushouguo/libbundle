/*
 * \file: sort.cpp
 * \brief: Created by hushouguo at 12:13:07 Oct 25 2018
 */

#include <stdio.h>

// o(n^2)
template <typename T>
void bubble_sort(T ary[], size_t len) {
	for (size_t i = 0; i < len - 1; ++i) {
		for (size_t j = 0; j < len - 1 - i; ++j) {
			if (ary[j] < ary[j + 1]) {
				T value = ary[j];
				ary[j] = ary[j + 1];
				ary[j + 1] = value;
			}
		}
	}
	printf("bubble sort: \n");
}

// o(n^2)
template <typename T>
void insert_sort(T ary[], size_t len) {
	for (size_t i = 1; i < len; ++i) {
		size_t curr_i = i;
		while (curr_i > 0) {
			size_t prev_i = curr_i - 1;
			if (ary[curr_i] > ary[prev_i]) {
				T value = ary[curr_i];
				ary[curr_i] = ary[prev_i];
				ary[prev_i] = value;
			}
			--curr_i;
		}
	}
	printf("insert sort: \n");
}

// o(n^2)
template <typename T>
void selection_sort(T ary[], size_t len) {
	for (size_t i = 0; i < len; ++i) {
		size_t max_i = i;
		for (size_t j = i + 1; j < len; ++j) {
			if (ary[j] > ary[max_i]) {
				max_i = j;
			}
		}

		T value = ary[i];
		ary[i] = ary[max_i];
		ary[max_i] = value;	
	}
	printf("selection sort: \n");
}

// o(n^1.3)
template <typename T>
void shell_sort(T ary[], size_t len) {
	printf("shell sort: \n");
}

// o(nlogn)
template <typename T>
void merge_sort(T ary[], size_t len) {
}

// o(nlogn)
template <typename T>
void quick_sort(T ary[], size_t len) {
}

//--------------------------------------

// o(n+k)
template <typename T>
void counting_sort(T ary[], size_t len) {
}

// o(n+k)
template <typename T>
void bucket_sort(T ary[], size_t len) {
}

// o(n*k)
template <typename T>
void radix_sort(T ary[], size_t len) {
}

template <typename T>
std::tuple<T, T> op_2(T ary[], size_t len) {
	assert(len == 2);
	return make_tuple(std::max(ary[0], ary[1]), std::min(ary[0], ary[1]));
}

template <typename T>
std::tuple<T, T, T, T> op_4(T ary[], size_t len) {
	assert(len == 4);
	T a, b, c, d;
	if (ary[0] >= ary[1]) {	// 1
		a = ary[0];
		b = ary[1];
	}
	else {
		a = ary[1];
		b = ary[0];
	}

	if (ary[2] >= ary[3]) { // 2
		c = ary[2];
		d = ary[3];
	}
	else {
		c = ary[3];
		d = ary[2];
	}

	if (a >= c) {	// 3
		if (b >= c) {
			
		}
		else if (b >= d) {
		}
		else {
		}
	}
	else if (a >= d) { // 4
		if (b >= d) {
		}
		else {
		}
	}
	else {
		return make_tuple(c, d, a, b);
	}

/*	
	std::tuple<T, T> t1 = op_2(ary, 2);
	std::tuple<T, T> t2 = op_2(&ary[2], 2);
	std::tuple<T, T> t3 = op_2(std::get<0>(t1), std::get<0>(t2));
*/
}

int main() {
	int ary[] = { 61, 17, 29, 22, 34, 60, 72, 21, 50, 1, 62 };
	for (auto value : ary) {
		printf("%d ", value);
	}
	printf("\n");
	//bubble_sort(ary, sizeof(ary) / sizeof(ary[0]));
	//insert_sort(ary, sizeof(ary) / sizeof(ary[0]));
	selection_sort(ary, sizeof(ary) / sizeof(ary[0]));
	for (auto value : ary) {
		printf("%d ", value);
	}
	printf("\n");
	return 0;
}
