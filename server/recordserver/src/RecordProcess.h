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
		bool request(u32 shard, u64 objectid, SOCKET s, const Netmessage* netmsg);
		void update();
		
	private:
		struct SlotDatabase {
			u32 shard = 0;
			u32 id = 0;
			bool isstop = false;
			MySQL* dbhandler = nullptr;
			std::thread* threadWorker = nullptr;
			std::mutex rlocker, wlocker;
			std::list<Recordmessage *> rlist;
			std::list<Recordmessage *> wlist;
			std::unordered_map<u64, Entity*> entities;
			struct FieldDescriptor {
				enum_field_types type;
				u32 flags;
				u64 length;
			};
			std::unordered_map<std::string, std::unordered_map<std::string, FieldDescriptor>> tables;
			SlotDatabase(u32 sid, u32 value) : shard(sid), id(value) {}
			inline void removeEntity(u64 objectid) {
				auto i = this->entities.find(objectid);
				if (i != this->entities.end()) {
					this->entities.erase(i);
				}
			}
			bool init(std::string conf);
			void stop();
			void update();
			u32 synchronous();
			bool loadField(std::string table);
			bool createTable(const char* table, const Entity* entity);
			bool alterTable(const char* table, const char* js, size_t length);
			bool addField(const char* table, const std::string& field_name, enum_field_types field_type);
			bool alterField(const char* table, const std::string& field_name, enum_field_types field_type);
			bool removeField(const char* table, const std::string& field_name);
			bool addKey(const char* table, const std::string& field_name);
			bool removeKey(const char* table, const std::string& field_name);
			bool serialize(const char* table, const Entity* entity);
			Entity* unserialize(const char* table, u64 objectid);
			bool removeEntity(const char* table, u64 objectid);
		};		
		std::vector<SlotDatabase*> _slots;		
		void update(SlotDatabase* slotDatabase);
};

#endif
