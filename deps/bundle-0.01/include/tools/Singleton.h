/*
 * \file: Singleton.h
 * \brief: Created by hushouguo at Jul 06 2017 20:17:52
 */
 
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

BEGIN_NAMESPACE_BUNDLE {
	template < typename T >	class Singleton {
			public:
				static T& getInstance() {
					return *instance;
				}
								
			protected:
				Singleton() {}
				~Singleton() {}
				
			private:
				Singleton(const Singleton &);
				Singleton& operator=(const Singleton &);

				static T* instance;
		};
}

#define INITIALIZE_INSTANCE(T, ...) template<> T* bundle::Singleton<T>::instance = new T(__VA_ARGS__)
#define DELETE_INSTANCE(T)	delete bundle::Singleton<T>::getInstance()

#endif
