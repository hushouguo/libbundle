/*
 * \file: ByteBuffer.h
 * \brief: Created by hushouguo at 15:13:26 Jan 19 2018
 */
 
#ifndef __BYTE_BUFFER_H__
#define __BYTE_BUFFER_H__

BEGIN_NAMESPACE_BUNDLE {
	class ByteBuffer
	{
		public:
			ByteBuffer(size_t size = 65535);
			ByteBuffer(const google::protobuf::Message* msg);
			~ByteBuffer();

		public:
			void append(const Byte* s, size_t len);
			template < typename T > void append(const T& value) {
				this->append((Byte*)&value, sizeof(value));	
			}

		public:
			ByteBuffer& operator<<(bool value);
			ByteBuffer& operator<<(u8 value);
			ByteBuffer& operator<<(u16 value);
			ByteBuffer& operator<<(u32 value);
			ByteBuffer& operator<<(u64 value);
			ByteBuffer& operator<<(s8 value);
			ByteBuffer& operator<<(s16 value);
			ByteBuffer& operator<<(s32 value);
			ByteBuffer& operator<<(s64 value);
			ByteBuffer& operator<<(float value);
			ByteBuffer& operator<<(double value);
			ByteBuffer& operator<<(std::string& value);
			ByteBuffer& operator<<(const std::string& value);
			ByteBuffer& operator<<(char* value);
			ByteBuffer& operator<<(const char* value);

		public:
			void read(Byte* out, size_t len);
			template < typename T > T read() {
				T value = * ((T const *) this->rbuffer());
				this->rlength(sizeof(T));
				return value;
			}

		public:
			ByteBuffer& operator>>(bool& value);
			ByteBuffer& operator>>(u8& value);
			ByteBuffer& operator>>(u16& value);
			ByteBuffer& operator>>(u32& value);
			ByteBuffer& operator>>(u64& value);
			ByteBuffer& operator>>(s8& value);
			ByteBuffer& operator>>(s16& value);
			ByteBuffer& operator>>(s32& value);
			ByteBuffer& operator>>(s64& value);
			ByteBuffer& operator>>(float& value);
			ByteBuffer& operator>>(double& value);
			
		public:
			size_t size() const;
			inline size_t capacity() const { return this->_capacity; }

			void clear();
			void reset();
			void reserve(size_t newsize);

		public:
			void rlength(size_t len);
			void wlength(size_t len);
			
			Byte* rbuffer() const;
			Byte* wbuffer(size_t reserve_size);

		public:
			bool CopyFrom(const google::protobuf::Message* msg);
			bool CopyTo(google::protobuf::Message* msg);
		
		private:
			Byte* _buf = nullptr;
			size_t _capacity = 0;
			size_t _rindex = 0, _windex = 0;
			size_t blank();
			void shrink();
	};
}

#endif
