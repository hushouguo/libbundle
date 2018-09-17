/*
 * \file: Entity.h
 * \brief: Created by hushouguo at 17:16:23 Sep 05 2018
 */
 
#ifndef __ENTITY_H__
#define __ENTITY_H__

BEGIN_NAMESPACE_BUNDLE {
	class EntityDescriptor;
	class Entity : public Entry<u64> {
		public:
			enum value_type {
				type_integer		=	0,	// 8 bytes signed integer, Json: Digital, Sql: long, int
				type_float			=	1,	// 4 bytes float, Json: Digital, Sql: float
				type_bool			=	2,	// 1 byte boolean, Json: bool, Sql: tinyint, tiny unsigned
				type_string			=	3,	// String, Json: String, Sql: autospan varchar | long text, varchar string 21845 utf-8, ascii: 65535
				type_max			=	4
			};

			static const char* ValueTypeName(value_type type);
			
			struct Value {
				value_type			type;
				s64					value_integer;
				float				value_float;
				bool				value_bool;
				std::string			value_string;
				Value() : type(type_string), value_string("UNDEFINED") {}
				Value(s8 value) : type(type_integer), value_integer(value) {}
				Value(u8 value) : type(type_integer), value_integer(value) {}
				Value(s16 value) : type(type_integer), value_integer(value) {}
				Value(u16 value) : type(type_integer), value_integer(value) {}
				Value(s32 value) : type(type_integer), value_integer(value) {}
				Value(u32 value) : type(type_integer), value_integer(value) {}
				Value(s64 value) : type(type_integer), value_integer(value) {}
				Value(u64 value) : type(type_integer), value_integer(value) {}
				Value(float value) : type(type_float), value_float(value) {}
				Value(double value) : type(type_float), value_float(static_cast<float>(value)) {}
				Value(bool value) : type(type_bool), value_bool(value) {}
				Value(char* value) : type(type_string), value_string(value) {}
				Value(const char* value) : type(type_string), value_string(value) {}
				Value(const std::string& value) : type(type_string), value_string(value) {}
				bool dirty = false;
				inline void set(s8 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(u8 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(s16 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(u16 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(s32 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(u32 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(s64 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(u64 value) { this->type = type_integer; this->value_integer = value; dirty = true; }
				inline void set(float value) { this->type = type_float; this->value_float = value; dirty = true; }
				inline void set(double value) { this->type = type_float; this->value_float = static_cast<float>(value); dirty = true; }
				inline void set(bool value) { this->type = type_bool; this->value_bool = value; dirty = true; }
				inline void set(char* value) { this->type = type_string; this->value_string = value; dirty = true; }
				inline void set(const char* value) { this->type = type_string; this->value_string = value; dirty = true; }
				inline void set(const std::string& value) { this->type = type_string; this->value_string = value; dirty = true; }
				template <typename T> void operator = (T value) { this->set(value); }
			};
	
		public:
			Entity(u64 entityid);
			const char* getClassName() override { return "Entity"; }

		public:
			// similar with google protobuf
			bool ParseFromString(const char* data, size_t length);
			bool SerializeToString(std::ostringstream& o, bool only_dirty = false);

		public:
			// similar with rapidjson
			bool HasMember(const char* name);
			
			bool IsNumber(const char* name);
			bool IsInteger(const char* name);
			bool IsFloat(const char* name);
			bool IsBool(const char* name);
			bool IsString(const char* name);

			Value& operator [](char* name);
			Value& operator [](const char* name);
			Value& operator [](const std::string& name);

			s64 GetInteger(const char* name);
			bool GetBool(const char* name);
			float GetFloat(const char* name);
			const std::string& GetString(const char* name);
			Value& GetValue(char* name);
			Value& GetValue(const char* name);
			Value& GetValue(const std::string& name);

			inline const std::unordered_map<std::string, Entity::Value>& values() const { return this->_values; }
			
		public:
			void dump();
			void clear();
			void clearDirty();
		
		private:
			std::unordered_map<std::string, Value> _values;
	};
}

#endif
