/*
 * \file: Entity.cpp
 * \brief: Created by hushouguo at 22:37:06 Sep 05 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	const char* Entity::ValueTypeName(value_type type) {
		static const char* __value_type_names[] = {
			[type_integer]	=	"integer",
			[type_float]	=	"float",
			[type_bool]		=	"bool",
			[type_string]	=	"string",
		};
		return __value_type_names[type];
  	}

	Entity::Entity(u64 entityid) : Entry<u64>(entityid) {
#if 0	
		auto& descriptor = sEntityDescriptor.getDescriptor(tableid);
		for (auto& i : descriptor) {
			value_type type = i.second;
			switch (type) {
				case type_s32: this->_values.insert(std::make_pair(i.first, Value(static_cast<s32>(0)))); break;
				case type_u32: this->_values.insert(std::make_pair(i.first, Value(static_cast<u32>(0)))); break;
				case type_s64: this->_values.insert(std::make_pair(i.first, Value(static_cast<s64>(0)))); break;
				case type_u64: this->_values.insert(std::make_pair(i.first, Value(static_cast<u64>(0)))); break;
				case type_float: this->_values.insert(std::make_pair(i.first, Value(static_cast<float>(0)))); break;
				case type_bool: this->_values.insert(std::make_pair(i.first, Value(false))); break;
				case type_string: this->_values.insert(std::make_pair(i.first, Value(""))); break;
				default: assert(false);
			}
		}
#endif		
	}
		
	bool Entity::ParseFromString(const char* data, size_t length) {
		rapidjson::Document root;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
		CHECK_RETURN(root.Parse(data, length).HasParseError() == false, false, "Parse `data` error");
		CHECK_RETURN(root.IsObject(), false, "`data` is not object");
		this->_values.clear();
		for (auto i = root.MemberBegin(); i != root.MemberEnd(); ++i) {
			rapidjson::Value& name = i->name;
			CHECK_RETURN(name.IsString(), false, "`name` is not string: %d", name.GetType());
			CHECK_RETURN(!ContainsKey(this->_values, name.GetString()), false, "duplicate `name`: %s", name.GetString());
			
			rapidjson::Value& value = i->value;
			CHECK_RETURN(!value.IsNull(), false, "`name`:%s is null", name.GetString());
			CHECK_RETURN(!value.IsObject(), false, "`name`:%s is object", name.GetString());
			CHECK_RETURN(!value.IsArray(), false, "`name`:%s is array", name.GetString());

			bool rc = false;
			if (value.IsNumber()) {
				if (value.IsDouble()) {
					rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetFloat()))).second;
				}
				else if (value.IsInt()) {
					rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetInt()))).second;
				}
				else if (value.IsUint()) {
					rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetUint()))).second;
				}
				else if (value.IsInt64()) {
					rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetInt64()))).second;
				}
				else if (value.IsUint64()) {
					rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetUint64()))).second;
				}
				else {
					CHECK_RETURN(false, false, "`name`:%s, unknown value number type: %d", name.GetString(), value.GetType());
				}
			}
			else if (value.IsString()) {
				rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.GetString()))).second;
			}
			else if (value.IsBool()) {
				rc = this->_values.insert(std::make_pair(name.GetString(), Value(value.IsTrue()))).second;
			}
			else {
				CHECK_RETURN(false, false, "`name`:%s, unknown value type: %d", name.GetString(), value.GetType());
			}
			assert(rc);			
		}		
		return true;
	}

	bool Entity::SerializeToString(std::ostringstream& o, bool only_dirty) {
		o.str("");
		o << "{\"id\":" << this->id;
		for (auto& i : this->_values) {
			const Value& value = i.second;
			if (!only_dirty || value.dirty) {
				o << ",\"" << i.first << "\":";
				switch (value.type) {					
					case type_integer: o << value.value_integer; break;
					// Note: For floating point numbers must preserve two decimal places.
					case type_float: o << std::fixed << std::setprecision(2) << value.value_float; break;
					case type_bool: o << (value.value_bool ? "true" : "false"); break;
					case type_string: o << "\"" << value.value_string << "\""; break;
					default: assert(false); break;
				}
			}
		}
		o << "}";
		return true;
	}

	void Entity::dump() {
		Trace << "Entity: " << this->id << ", attributes size: " << this->_values.size();
		for (auto& i : this->_values) {
			const Value& value = i.second;
			switch (value.type) {				
				case type_integer: Trace << " " << i.first << ": " << value.value_integer << (value.dirty ? " *" : ""); break;
				case type_float: Trace << " " << i.first << ": " << value.value_float << (value.dirty ? " *" : ""); break;
				case type_bool: Trace << " " << i.first << ": " << (value.value_bool ? "true" : "false") << (value.dirty ? " *" : ""); break;
				case type_string: Trace << " " << i.first << ": " << "\"" << value.value_string << "\"" << (value.dirty ? " *" : ""); break;
				default: assert(false); break;
			}
		}
	}

	void Entity::clear() {
		this->_values.clear();
	}
	
	void Entity::clearDirty() {
		for (auto& i : this->_values) {
			Value& value = i.second;
			value.dirty = false;
		}
	}

	bool Entity::HasMember(const char* name) {
		return ContainsKey(this->_values, name);
	}

#define CHECK_VALUE_TYPE(name, value_type)\
	({\
		auto __i = this->_values.find(name);\
		bool __rc = __i != this->_values.end();\
		if (__rc) {\
			const Value& __value = __i->second;\
			__rc = __value.type == value_type;\
		}\
		else { \
			Alarm << "not exist name: " << name; \
		}\
		__rc;\
	})

	bool Entity::IsNumber(const char* name) {
		return IsInteger(name) || IsFloat(name);
	}

	bool Entity::IsInteger(const char* name) {
		return CHECK_VALUE_TYPE(name, type_integer);
	}
	
	bool Entity::IsFloat(const char* name) {
		return CHECK_VALUE_TYPE(name, type_float);
	}
		
	bool Entity::IsBool(const char* name) {
		return CHECK_VALUE_TYPE(name, type_bool);
	}
	
	bool Entity::IsString(const char* name) {
		return CHECK_VALUE_TYPE(name, type_string);
	}

	Entity::Value& Entity::operator [](char* name) {
		return this->_values[name];
	}
	Entity::Value& Entity::operator [](const char* name) {
		return this->_values[name];
	}
	Entity::Value& Entity::operator [](const std::string& name) {
		return this->_values[name];
	}
	

#define GET_VALUE(NAME, TYPE)\
	({\
		auto __i = this->_values.find(NAME);\
		assert(__i != this->_values.end());\
		const Value& __value = __i->second;\
		assert(__value.type == type_##TYPE);\
		return __value.value_##TYPE;\
	})

	s64 Entity::GetInteger(const char* name) {
		GET_VALUE(name, integer);
	}
	
	float Entity::GetFloat(const char* name) {
		GET_VALUE(name, float);
	}
	
	bool Entity::GetBool(const char* name) {
		GET_VALUE(name, bool);
	}
	
	const std::string& Entity::GetString(const char* name) {
		GET_VALUE(name, string);
	}

	Value& Entity::GetValue(char* name) {
		return this->_values[name];
	}
	
	Value& Entity::GetValue(const char* name) {
		return this->_values[name];
	}
	
	Value& Entity::GetValue(const std::string& name) {
		return this->_values[name];
	}
}

