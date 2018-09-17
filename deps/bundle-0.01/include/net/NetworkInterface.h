/*
 * \file: NetworkInterface.h
 * \brief: Created by hushouguo at 16:19:17 Aug 25 2018
 */
 
#ifndef __NETWORK_INTERFACE_H__
#define __NETWORK_INTERFACE_H__

BEGIN_NAMESPACE_BUNDLE {
	class NetworkInterface {
		public:
			virtual SOCKET fd() = 0;
			virtual void sendMessage(const Netmessage*) = 0;
			virtual void sendMessage(s32, const google::protobuf::Message*) = 0;
			virtual void sendMessage(const void* data, size_t length) = 0;
	};
}

#endif
