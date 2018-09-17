/*
 * \file: RecordClient.h
 * \brief: Created by hushouguo at 19:58:05 Aug 24 2018
 */
 
#ifndef __RECORDCLIENT_H__
#define __RECORDCLIENT_H__

BEGIN_NAMESPACE_BUNDLE {
	class RecordClient : public NetworkClient {
		public:
			RecordClient();
			
		public:
			void serialize(u32, std::string, u64, const std::string&, std::function<void(u32, std::string, u64, u32)>);
			void unserialize(u32, std::string, u64, std::function<void(u32, std::string, u64, u32, const char*, size_t)>);

			//delete and select

		private:
			std::list<std::function<void(u32, std::string, u64, u32)>> _serializeCallbacks;
			std::list<std::function<void(u32, std::string, u64, u32, const char*, size_t)>> _unserializeCallbacks;
			bool msgParser(NetworkInterface* task, const Netmessage* netmsg);
	};
}

#endif
