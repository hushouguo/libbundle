/*
 * \file: Socket.h
 * \brief: Created by hushouguo at 16:29:33 Jun 21 2018
 */
 
#ifndef __SOCKET_H__
#define __SOCKET_H__

#define COUNT_MESSAGE_SPAN	60

BEGIN_NAMESPACE_BUNDLE {
	class Socket {
		public:
			Socket(SOCKET connfd, std::function<int(const Byte*, size_t)> splitMessage) : _fd(connfd) {
				nonblocking(this->fd());
				this->_splitMessage = splitMessage;
			}

			~Socket() {
				this->close();
				for (auto& msg : this->_sendlist) {
					releaseMessage(msg);
				}
				this->_sendlist.clear();
			}
			
		public:
			inline SOCKET fd() { return this->_fd; }			
			void close();
			inline u64 lastSecond() { return this->_lastSecond; }
			inline u32 totalMessage() { return this->_totalMessage; }
			u32 recentMessage(u32 seconds);

		public:
			bool receiveMessage(Socketmessage*& msg);
			bool sendMessage();
			bool sendMessage(Socketmessage* msg);
			
		private:
			SOCKET _fd = BUNDLE_INVALID_SOCKET;
			ByteBuffer _rbuffer, _wbuffer;
			std::list<Socketmessage*> _sendlist;
			std::function<int(const Byte*, size_t)> _splitMessage = nullptr;
			bool splitMessage(Socketmessage*& msg);
			
			u64 _lastSecond = timeSecond();
			u32 _countMessage[COUNT_MESSAGE_SPAN];
			u32 _totalMessage = 0;
			void receivedMessage();
	};	
}

#endif
