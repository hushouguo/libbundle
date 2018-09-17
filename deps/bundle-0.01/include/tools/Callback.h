/*
 * \file: Callback.h
 * \brief: Created by hushouguo at Jul 06 2017 20:17:11
 */
 
#ifndef __CALLBACK_H__
#define __CALLBACK_H__

BEGIN_NAMESPACE_BUNDLE {
	template <typename T, typename R = bool, typename P1 = void, typename P2 = void, typename P3 = void, typename P4 = void, typename P5 = void, typename P6 = void, typename P7 = void>
		class Callback {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) = 0;
		};

	template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		class Callback < T, R, P1, P2, P3, P4, P5, P6> {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) = 0;
		};

	template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
		class Callback < T, R, P1, P2, P3, P4, P5> {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
		};

	template <typename T, typename R, typename P1, typename P2, typename P3, typename P4>
		class Callback < T, R, P1, P2, P3, P4> {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2, P3 p3, P4 p4) = 0;
		};

	template <typename T, typename R, typename P1, typename P2, typename P3>
		class Callback < T, R, P1, P2, P3> {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2, P3 p3) = 0;
		};

	template <typename T, typename R , typename P1, typename P2>
		class Callback < T, R, P1, P2 > {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P1 p1, P2 p2) = 0;
		};

	template <typename T, typename R, typename P>
		class Callback < T, R, P > {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry, P p) = 0;
		};

	template <typename T, typename R >
		class Callback < T, R > {
			public:
				virtual ~Callback() {}
				virtual R invoke(T *entry) = 0;
		};

	template <typename T >
		class Callback < T > {
			public:
				virtual ~Callback() {}
				virtual bool invoke(T *entry) = 0;
		};
};

#endif
