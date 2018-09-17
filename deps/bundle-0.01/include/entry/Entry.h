/*
 * \file: Entry.h
 * \brief: Created by hushouguo at Jul 06 2017 20:02:22
 */
 
#ifndef __ENTRY_H__
#define __ENTRY_H__

BEGIN_NAMESPACE_BUNDLE {
	template<typename ID, typename DATA = void> class Entry {
		public:
			Entry(ID i, DATA d) : id(i), data(d) {}
			virtual ~Entry() {}
		public:
			typedef ID ID_TYPE;
			typedef DATA DATA_TYPE;
			virtual const char* getClassName() = 0;
		public:
			ID id;
			DATA data;
	};

	template<typename ID> class Entry<ID, char*> {
		public:
			Entry(ID i, char* d) : id(i) {
				this->data = strdup(d);
			}
			virtual ~Entry() {
				SafeDelete(this->data);
			}
		public:
			typedef ID ID_TYPE;
			typedef char* DATA_TYPE;
			virtual const char* getClassName() = 0;
		public:
			ID id;
			char* data;
	};

	template<typename ID> class Entry<ID, const char*> {
		public:
			Entry(ID i, const char* d) : id(i), data(d) {}
			virtual ~Entry() {}
		public:
			typedef ID ID_TYPE;
			typedef const char* DATA_TYPE;
			virtual const char* getClassName() = 0;
		public:
			ID id;
			const char* data;
	};

	template<typename ID> class Entry<ID, void> {
		public:
			Entry(ID i) : id(i) {}
			virtual ~Entry() {}
		public:
			typedef ID ID_TYPE;
			typedef void DATA_TYPE;
			virtual const char* getClassName() = 0;
		public:
			ID id;
	};
};

#endif
