/*
 * \file: Constructor.h
 * \brief: Created by hushouguo at 19:45:26 Jan 21 2018
 */
 
#ifndef __CONSTRUCTOR_H__
#define __CONSTRUCTOR_H__

BEGIN_NAMESPACE_BUNDLE {
	template <typename T>
	T* Constructor(T* InPlace)
#ifdef PLATFORM_LINUX
	{ return new (InPlace) T();	}
#endif
#ifdef PLATFORM_WINDOWS
	{ return new (InPlace) T;	}
#endif


	template <typename T, typename P>
		T* Constructor(T* InPlace, P p)
		{ return new (InPlace) T(p); }

	template <typename T, typename P1, typename P2>
		T* Constructor(T* InPlace, P1 p1, P2 p2)
		{ return new (InPlace) T(p1, p2); }

	template <typename T, typename P1, typename P2, typename P3>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3)
		{ return new (InPlace) T(p1, p2, p3); }

	template <typename T, typename P1, typename P2, typename P3, typename P4>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3, P4 p4)
		{ return new (InPlace) T(p1, p2, p3, p4); }

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{ return new (InPlace) T(p1, p2, p3, p4, p5); }

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{ return new (InPlace) T(p1, p2, p3, p4, p5, p6); }

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
		{ return new (InPlace) T(p1, p2, p3, p4, p5, p6, p7); }

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
		T* Constructor(T* InPlace, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
		{ return new (InPlace) T(p1, p2, p3, p4, p5, p6, p7, p8); }
}

#endif
