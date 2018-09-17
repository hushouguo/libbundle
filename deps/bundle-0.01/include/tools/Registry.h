/*
 * \file: Registry.h
 * \brief: Created by hushouguo at 18:41:20 Aug 31 2018
 */
 
#ifndef __REGISTRY_H__
#define __REGISTRY_H__

BEGIN_NAMESPACE_BUNDLE {
	class Registry {
		public:
			bool set(const char* key, bool value);
			bool set(const char* key, u8 value);
			bool set(const char* key, u16 value);
			bool set(const char* key, u32 value);
			bool set(const char* key, u64 value);
			bool set(const char* key, s8 value);
			bool set(const char* key, s16 value);
			bool set(const char* key, s32 value);
			bool set(const char* key, s64 value);
			bool set(const char* key, float value);
			bool set(const char* key, double value);
			bool set(const char* key, char* value);
			bool set(const char* key, const char* value);
			bool set(const char* key, std::string& value);
			bool set(const char* key, const std::string& value);

		public:
			bool get(const char* key, bool defaultValue);
			u8 get(const char* key, u8 defaultValue);
			u16 get(const char* key, u16 defaultValue);
			u32 get(const char* key, u32 defaultValue);
			u64 get(const char* key, u64 defaultValue);
			s8 get(const char* key, s8 defaultValue);
			s16 get(const char* key, s16 defaultValue);
			s32 get(const char* key, s32 defaultValue);
			s64 get(const char* key, s64 defaultValue);
			float get(const char* key, float defaultValue);
			double get(const char* key, double defaultValue);
			char* get(const char* key, char* defaultValue);
			const char* get(const char* key, const char* defaultValue);

		public:
			bool findKey(const char* key);
			void removeKey(const char* key);

		public:
			void clear();
			void dump();

		private:
			std::unordered_map<std::string, std::string> _values;
			bool setValue(const char* key, const char* value);
			const char* getValue(const char* key);
	};
}

#endif
