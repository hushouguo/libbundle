/*
 * \file: bubble.cpp
 * \brief: Created by hushouguo at 12:13:07 Oct 25 2018
 */

#include <stdio.h>

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
}

int main() {
	int ary[] = { 61, 17, 29, 22, 34, 60, 72, 21, 50, 1, 62 };
	for (auto value : ary) {
		printf("%d ", value);
	}
	printf("\n");
	bubble_sort(ary, sizeof(ary) / sizeof(ary[0]));
	for (auto value : ary) {
		printf("%d ", value);
	}
	printf("\n");
	return 0;
}
