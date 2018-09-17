/*
 * \file: NetworkClient.cpp
 * \brief: Created by hushouguo at 08:43:42 Sep 01 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {			
	SOCKET NetworkClient::fd() {
		return this->socketClient() ? this->socketClient()->fd() : BUNDLE_INVALID_SOCKET;
	}

	void NetworkClient::sendMessage(const Netmessage* netmsg) {
		if (this->socketClient()) {
			this->socketClient()->sendMessage(netmsg, netmsg->len);
		}
	}

	void NetworkClient::sendMessage(s32 msgid, const google::protobuf::Message* msg) {
		if (this->socketClient()) {
			int dataSize = msg->ByteSize();
			char buffer[sizeof(Netmessage) + dataSize];
			Netmessage* netmsg = (Netmessage *) buffer;
			netmsg->len = sizeof(Netmessage) + dataSize;
			netmsg->id = msgid;
			netmsg->flags = 0;
			netmsg->timestamp = 0;
			bool rc = msg->SerializeToArray(netmsg->payload, dataSize);
			CHECK_RETURN(rc, void(0), "SerializeToArray error");
			this->socketClient()->sendMessage(netmsg, netmsg->len);
		}
	}

	void NetworkClient::sendMessage(const void* payload, size_t payload_len) {
		this->socketClient()->sendMessage(payload, payload_len);
	}

	bool NetworkClient::connect(const char* address, int port) {
		SafeDelete(this->_socketClient);
		this->_socketClient = SocketClientCreator::create([](const Byte* buffer, size_t len) -> int {
				Netmessage* netmsg = (Netmessage*) buffer;
				return len < sizeof(Netmessage) || len < netmsg->len ? 0 : netmsg->len;
				});

		bool rc = this->_socketClient->connect(address, port, true);
		if (!rc) {
			SafeDelete(this->_socketClient);
			return false;
		}

		Trace.cout("NetworkClient establish with: %s:%d", address, port);
		return true;
	}

	bool NetworkClient::update() {
		CHECK_RETURN(this->_socketClient, false, "please call `connect` to init client");
		while (!this->isstop()) {
			bool is_establish = false, is_close = false;
			bool rc = true;
			Rawmessage* rawmsg = this->_socketClient->receiveMessage(is_establish, is_close);
			if (rawmsg) {
				if (is_establish) {
					if (this->_establishConnection) {
						this->_establishConnection(this);
					}
				}
				else if (is_close) {
					if (this->_lostConnection) {
						this->_lostConnection(this);
					}
				}
				else {
					if (this->_msgParser) {
						const Netmessage* netmsg = (Netmessage*)rawmsg->payload;
						assert(rawmsg->payload_len == netmsg->len);
						rc = this->_msgParser(this, netmsg);
					}
				}
				if (rc) {
					this->socketClient()->releaseMessage(rawmsg);
				}
			}
			else {
				return true;
			}
		}
		return !this->isstop();
	}

	void NetworkClient::releaseMessage(const Netmessage* netmsg) {
		Rawmessage* rawmsg = (Rawmessage *) ((Byte*) netmsg - offsetof(Rawmessage, payload));
		assert(rawmsg->payload_len == netmsg->len);
		assert(this->socketClient());
		this->socketClient()->releaseMessage(rawmsg);
	}

	void NetworkClient::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_socketClient) {
				this->_socketClient->stop();
			}
		}
	}

	NetworkClient::~NetworkClient() {
		this->stop();
	}
}
