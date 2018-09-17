/*
 * \file: EntityDescriptor.h
 * \brief: Created by hushouguo at 08:14:39 Sep 06 2018
 */
 
#ifndef __ENTITYDESCRIPTOR_H__
#define __ENTITYDESCRIPTOR_H__

BEGIN_NAMESPACE_BUNDLE {
	class EntityDescriptor {
		public:
			bool loadDescriptor(u32 tableid, const char* filename);
			const std::unordered_map<std::string, Entity::value_type>& getDescriptor(u32 tableid);

		private:
			std::unordered_map<u32, std::unordered_map<std::string, Entity::value_type>> _tables;
	};
}

#define sEntityDescriptor bundle::Singleton<bundle::EntityDescriptor>::getInstance()

#endif
