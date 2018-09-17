/*
 * \file: Manager.h
 * \brief: Created by hushouguo at Jul 06 2017 20:09:47
 */
 
#ifndef __MANAGER_H__
#define __MANAGER_H__

BEGIN_NAMESPACE_BUNDLE {
	template<class ENTRY> class Manager {
		public:
			bool add(ENTRY* entry) {
				return this->__entries.insert(std::make_pair(entry->id, entry)).second;
			}

			void remove(ENTRY* entry) {
				auto i = this->__entries.find(entry->id);
				CHECK_RETURN(i != this->__entries.end(), void(0), "Not found entry:%s", entry->getClassName());
				this->__entries.erase(i);
			}

			size_t size() {
				return this->__entries.size();
			}

			bool empty() {
				return this->__entries.empty();
			}

			void clear() {
				this->__entries.clear();
			}

			ENTRY* find(typename ENTRY::ID_TYPE id) {
				auto i = this->__entries.find(id);
				return i == this->__entries.end() ? nullptr : i->second;
			}

			bool traverse(Callback<ENTRY>& eee) {
				for (auto& entry : this->__entries) {
					if (!eee.invoke(entry.second)) {
						return false;
					}
				}
				return true;
			}

			ENTRY* pop_front() {
				auto i = this->__entries.begin();
				ENTRY* entry = i->second;
				this->__entries.erase(i);
				return entry;
			}

		private:
			std::unordered_map<typename ENTRY::ID_TYPE, ENTRY*> __entries;
	};
}

#endif
