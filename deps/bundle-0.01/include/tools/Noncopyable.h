/*
 * \file: Noncopyable.h
 * \brief: Created by hushouguo at 13:25:54 Aug 08 2018
 */
 
#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

BEGIN_NAMESPACE_BUNDLE {
	class Noncopyable
	{
		private:
			Noncopyable& operator=( const Noncopyable &rhs );
			Noncopyable( const Noncopyable &rhs);

		protected:
			Noncopyable() {}
			~Noncopyable() {}
	};
}

#endif
