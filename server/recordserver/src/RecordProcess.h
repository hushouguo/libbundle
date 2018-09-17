/*
 * \file: RecordProcess.h
 * \brief: Created by hushouguo at 03:15:05 Sep 07 2018
 */
 
#ifndef __RECORD_PROCESS_H__
#define __RECORD_PROCESS_H__

class RecordProcess : public Entry<u32, std::string> {
	public:
		RecordProcess(u32, const char*);
		const char* getClassName() override { return "RecordProcess"; }

	public:
		bool init(const std::vector<std::string>& v);
		void stop();

	public:
		bool request(u32 shard, u32 tableid, u64 objectid, SOCKET s, Netmessage* netmsg);
		void update();
		
	private:
		struct SlotDatabase {
			u32 id = 0;
			bool isstop = false;
			MySQL* dbhandler = nullptr;
			std::thread* threadWorker = nullptr;
			std::mutex rlocker, wlocker;
			std::list<Recordmessage *> rlist;
			std::list<Recordmessage *> wlist;
			std::unordered_map<u64, Entity*> entities;
			std::unordered_map<std::string, std::unordered_map<std::string, Entity::value_type>> tables;
			SlotDatabase(u32 value) : id(value) {}
			bool init(std::string conf);
			//bool loadField(u32 shard, std::string table);
			void stop();
			void update();
			u32 synchronous();
			void response(SOCKET s, Netmessage* netmsg);
		};		
		std::vector<SlotDatabase*> _slots;		
		void update(SlotDatabase* slotDatabase);
};

#endif
