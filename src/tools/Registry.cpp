/*
 * \file: Registry.cpp
 * \brief: Created by hushouguo at 18:51:01 Aug 31 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	bool Registry::setValue(const char* key, const char* value) {
		return this->_values.insert(std::make_pair(key, value)).second;
	}

#define SET_VALUE(KEY, VALUE_FORMAT_STRING, VALUE)\
	char buf[1024];\
	snprintf(buf, sizeof(buf), VALUE_FORMAT_STRING, VALUE);\
	return setValue(key, buf);
	
	bool Registry::set(const char* key, bool value) {
		SET_VALUE(key, "%s", value ? "true" : "false");
	}

	bool Registry::set(const char* key, u8 value) {
		SET_VALUE(key, "%u", value);
	}

	bool Registry::set(const char* key, u16 value) {
		SET_VALUE(key, "%u", value);
	}

	bool Registry::set(const char* key, u32 value) {
		SET_VALUE(key, "%u", value);
	}

	bool Registry::set(const char* key, u64 value) {
		SET_VALUE(key, "%lu", value);
	}

	bool Registry::set(const char* key, s8 value) {
		SET_VALUE(key, "%d", value);
	}

	bool Registry::set(const char* key, s16 value) {
		SET_VALUE(key, "%d", value);
	}

	bool Registry::set(const char* key, s32 value) {
		SET_VALUE(key, "%d", value);
	}

	bool Registry::set(const char* key, s64 value) {
		SET_VALUE(key, "%ld", value);
	}

	bool Registry::set(const char* key, float value) {
		SET_VALUE(key, "%f", value);
	}

	bool Registry::set(const char* key, double value) {
		SET_VALUE(key, "%lf", value);
	}

	bool Registry::set(const char* key, char* value) {
		SET_VALUE(key, "%s", value);
	}

	bool Registry::set(const char* key, const char* value) {
		SET_VALUE(key, "%s", value);
	}

	bool Registry::set(const char* key, std::string& value) {
		SET_VALUE(key, "%s", value.c_str());
	}

	bool Registry::set(const char* key, const std::string& value) {
		SET_VALUE(key, "%s", value.c_str());	
	}

	//-------------------------------------------------------------
	//
	
	const char* Registry::getValue(const char* key) {
		auto i = this->_values.find(key);
		return i == this->_values.end() ? nullptr : i->second.c_str();
	}

	bool Registry::get(const char* key, bool defaultValue) {
		const char* value = this->getValue(key);
		return value ? (strcasecmp(value, "true") == 0) : defaultValue;
	}

	u8 Registry::get(const char* key, u8 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtoul(value, nullptr, 10) : defaultValue;
	}

	u16 Registry::get(const char* key, u16 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtoul(value, nullptr, 10) : defaultValue;
	}

	u32 Registry::get(const char* key, u32 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtoul(value, nullptr, 10) : defaultValue;
	}

	u64 Registry::get(const char* key, u64 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtoull(value, nullptr, 10) : defaultValue;
	}

	s8 Registry::get(const char* key, s8 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtol(value, nullptr, 10) : defaultValue;
	}

	s16 Registry::get(const char* key, s16 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtol(value, nullptr, 10) : defaultValue;
	}

	s32 Registry::get(const char* key, s32 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtol(value, nullptr, 10) : defaultValue;
	}

	s64 Registry::get(const char* key, s64 defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtoll(value, nullptr, 10) : defaultValue;
	}

	float Registry::get(const char* key, float defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtof(value, nullptr) : defaultValue;
	}

	double Registry::get(const char* key, double defaultValue) {
		const char* value = this->getValue(key);
		return value ? strtod(value, nullptr) : defaultValue;
	}

	char* Registry::get(const char* key, char* defaultValue) {
		const char* value = this->getValue(key);
		return value ? (char*)value : defaultValue;
	}

	const char* Registry::get(const char* key, const char* defaultValue) {
		const char* value = this->getValue(key);
		return value ? value : defaultValue;
	}
	
	//-------------------------------------------------------------
	//
			
	bool Registry::findKey(const char* key) {
		return ContainsKey(this->_values, key);
	}

	void Registry::removeKey(const char* key) {
		auto i = this->_values.find(key);
		if (i != this->_values.end()) {
			this->_values.erase(i);
		}
	}
	
	void Registry::clear() {
		this->_values.clear();
	}

	void Registry::dump() {
		Trace << "all registry:";
		for (auto& i : this->_values) {
			Trace << "    " << i.first << ": " << i.second;
		}
	}
}
