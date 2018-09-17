/*
 * \file: ByteBuffer.cpp
 * \brief: Created by hushouguo at 15:15:33 Jan 19 2018
 */

#include "bundle.h"

#define BUFFER_CHUNK_SIZE	0x200

BEGIN_NAMESPACE_BUNDLE {
	ByteBuffer::ByteBuffer(size_t size) {
		this->reserve(size);
	}

	ByteBuffer::ByteBuffer(const google::protobuf::Message* msg) : ByteBuffer() {
		this->CopyFrom(msg);
	}

	ByteBuffer::~ByteBuffer() {
		this->clear();
	}

	void ByteBuffer::append(const Byte* s, size_t len) {
		memcpy(this->wbuffer(len), s, len);
		this->wlength(len);
	}

	ByteBuffer& ByteBuffer::operator << (bool value) {
		this->append<bool>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (u8 value) {
		this->append<u8>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (u16 value) {
		this->append<u16>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (u32 value) {
		this->append<u32>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (u64 value) {
		this->append<u64>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (s8 value) {
		this->append<s8>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (s16 value) {
		this->append<s16>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (s32 value) {
		this->append<s32>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (s64 value) {
		this->append<s64>(value);
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (float value) {
		this->append<float>(value);
		return *this;
	}

	ByteBuffer& ByteBuffer::operator << (double value) {
		this->append<double>(value);
		return *this;
	}

	ByteBuffer& ByteBuffer::operator << (std::string& value) {
		this->append((const Byte*) value.data(), value.length());
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (const std::string& value) {
		this->append((const Byte*) value.data(), value.length());
		return *this;
	}

	ByteBuffer& ByteBuffer::operator << (char* value) {
		if (value != nullptr) {
			this->append((const Byte*) value, strlen(value));
		}
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator << (const char* value) {
		if (value != nullptr) {
			this->append((const Byte*) value, strlen(value));
		}
		return *this;
	}


	void ByteBuffer::read(Byte* out, size_t len) {
		memcpy(out, this->rbuffer(), len);
		this->rlength(len);
	}

	ByteBuffer& ByteBuffer::operator >> (bool& value) {
		value = this->read<bool>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (u8& value) {
		value = this->read<u8>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (u16& value) {
		value = this->read<u16>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (u32& value) {
		value = this->read<u32>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (u64& value) {
		value = this->read<u64>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (s8& value) {
		value = this->read<s8>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (s16& value) {
		value = this->read<s16>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (s32& value) {
		value = this->read<s32>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (s64& value) {
		value = this->read<s64>();
		return *this;
	}
	
	ByteBuffer& ByteBuffer::operator >> (float& value) {
		value = this->read<float>();
		return *this;
	}

	ByteBuffer& ByteBuffer::operator >> (double& value) {
		value = this->read<double>();
		return *this;
	}
	

	size_t ByteBuffer::size() const {
		assert(this->_windex >= this->_rindex);
		return this->_windex - this->_rindex;
	}

	void ByteBuffer::clear() {
		SafeFree(this->_buf);
		this->_capacity = this->_rindex = this->_windex = 0;
	}

	void ByteBuffer::reset() {
		this->_rindex = this->_windex = 0;
	}

	void ByteBuffer::reserve(size_t newsize) {
		if (blank() > newsize) {
			return;
		}

		if (newsize < BUFFER_CHUNK_SIZE) {
			newsize = BUFFER_CHUNK_SIZE;
		}

		newsize += this->_capacity;

		this->_buf = (Byte*) ::realloc(this->_buf, newsize);
		this->_capacity = newsize;
	}

	void ByteBuffer::rlength(size_t len) {
		this->_rindex += len;
		assert(this->_rindex <= this->_windex);
		this->shrink();
	}
	
	void ByteBuffer::wlength(size_t len) {
		this->_windex += len;
		assert(this->_windex <= this->_capacity);
	}

	Byte* ByteBuffer::rbuffer() const { 
		return &this->_buf[this->_rindex]; 
	}
	
	Byte* ByteBuffer::wbuffer(size_t reserve_size) {
		this->reserve(reserve_size);
		return &this->_buf[this->_windex];
	}

	size_t ByteBuffer::blank() {
		return this->_windex >= this->_capacity ? 0 : this->_capacity - this->_windex;
	}

	void ByteBuffer::shrink() {
		if (this->size() == 0) {
			this->reset();
		}
	}

	bool ByteBuffer::CopyFrom(const google::protobuf::Message* msg) {
		int size = msg->ByteSize();
		bool rc = msg->SerializeToArray(this->wbuffer(size), size);
		if (rc) {
			this->wlength(size);
		}
		return rc;
	}

	bool ByteBuffer::CopyTo(google::protobuf::Message* msg) {
		return msg->ParseFromArray(this->rbuffer(), this->size());
	}
}

