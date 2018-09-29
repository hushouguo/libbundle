/*
 * \file: Socket.cpp
 * \brief: Created by hushouguo at 16:29:36 Jun 21 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"
#include "WorkerProcess.h"

BEGIN_NAMESPACE_BUNDLE {
	bool Socket::splitMessage(Socketmessage*& msg) {
		CHECK_RETURN(this->_slotWorker != nullptr, false, "splitMessage unavailable");
		msg = nullptr;
		int rc = this->_slotWorker->splitMessage()(this->_rbuffer.rbuffer(), this->_rbuffer.size());
		if (rc == 0) {
			return true; 	// incomplete message
		}
		else if (rc < 0) {
			return false; 	// illegal message
		}
		
		//assert(size_t(rc) <= this->_rbuffer.size());
		CHECK_RETURN(size_t(rc) <= this->_rbuffer.size(), false, "rc: %d overflow rbuffer size: %ld", rc, this->_rbuffer.size());
		msg = allocateMessage(this->fd(), SM_OPCODE_MESSAGE, this->_rbuffer.rbuffer(), size_t(rc));
		this->_rbuffer.rlength(size_t(rc));
		return true;
	}

	void Socket::receivedMessage() {
		u64 nowtime = timeSecond();
		this->_lastSecond = nowtime;
		u32 mod = nowtime % COUNT_MESSAGE_SPAN;
		this->_countMessage[mod]++;
		this->_totalMessage++;
	}

	u32 Socket::recentMessage(u32 seconds) {
		u32 total = 0;
		u64 nowtime = timeSecond();
		for (u32 n = 0; n < seconds; ++n) {
			total += this->_countMessage[(nowtime - n) % COUNT_MESSAGE_SPAN];
		}
		return total;
	}
	
	bool Socket::receiveMessage(Socketmessage*& msg) {
		msg = nullptr;
		while (true) {
			bool rc = this->splitMessage(msg);
			if (!rc) {
				return false;
			}

			if (msg) {
				this->receivedMessage();
				return true;
			}

			size_t readlen = 960;
			ssize_t n = TEMP_FAILURE_RETRY(
					::recv(this->_fd, this->_rbuffer.wbuffer(readlen), readlen, MSG_DONTWAIT | MSG_NOSIGNAL));
			
			if (n == 0) {
				//Error.cout("lost Connection: %d", this->_fd);
				return false; // lost connection
			}
			else if (n < 0) {
				if (interrupted()) {
					continue;
				}
				
				if (wouldblock()) {
					break; // no more data to read
				}
				
				CHECK_RETURN(false, false, "socket receive error: %d, %s", errno, strerror(errno));
			}
			else {
				this->_rbuffer.wlength(size_t(n));
			}			
		}
		return true;
	}

	bool Socket::sendMessage() {
		while (true) {
			const Socketmessage* msg = nullptr;
			const Byte* buffer = nullptr;
			size_t len = 0;
			if (this->_wbuffer.size() > 0) {
				buffer = this->_wbuffer.rbuffer();
				len = this->_wbuffer.size();
			}
			else if (!this->_sendlist.empty()) {
				msg = this->_sendlist.front();
				this->_sendlist.pop_front();
				buffer = msg->payload;
				len = msg->payload_len;
			}
			else { return true; }

			ssize_t sendsize = TEMP_FAILURE_RETRY(::send(this->_fd, buffer, len, MSG_DONTWAIT | MSG_NOSIGNAL));
		
			if (sendsize == 0) {
				//Error.cout("lost Connection:%d", this->_fd);
				return false; // lost connection
			}
			else if (sendsize < 0) {
				if (interrupted()) { 
					continue; 
				}
		
				if (wouldblock()) {
					break; // socket buffer is full
				}
	
				CHECK_RETURN(false, false, "send error: %d, %s", errno, strerror(errno));
			}
			else {
				if (this->_wbuffer.size() > 0) {
					this->_wbuffer.rlength(size_t(sendsize));
				}
				else {
					if (size_t(sendsize) < len) {
						assert(this->_wbuffer.size() == 0);
						this->_wbuffer.append(buffer + size_t(sendsize), len - size_t(sendsize));
					}
					assert(msg);
					bundle::releaseMessage(msg);
				}
			}
		}
		return true;	
	}
	
	bool Socket::sendMessage(const Socketmessage* msg) {
		assert(msg);
		assert(msg->magic == MAGIC);
		assert(msg->s == this->fd());
		assert(msg->payload_len > 0);
		this->_sendlist.push_back(msg);
		return this->sendMessage();
	}

	void Socket::close() {
		SafeClose(this->_fd);
	}	
}


