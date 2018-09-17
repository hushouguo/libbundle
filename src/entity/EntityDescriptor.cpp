/*
 * \file: EntityDescriptor.cpp
 * \brief: Created by hushouguo at 08:23:24 Sep 06 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	bool EntityDescriptor::loadDescriptor(u32 tableid, const char* filename) {
		XmlParser xmlParser;
		if (!xmlParser.open(filename)) {
			return false;
		}

		auto& descriptor = this->_tables[tableid];
		descriptor.clear();

		std::unordered_map<std::string, Entity::value_type> types {
			{ "integer",  Entity::type_integer },
			{ "float", Entity::type_float },
			{ "bool", Entity::type_bool },
			{ "string", Entity::type_string },
		};

		XmlParser::XML_NODE root = xmlParser.getRootNode();
		CHECK_RETURN(root, false, "not found root node");

		for (XmlParser::XML_NODE node = xmlParser.getChildNode(root, "attribute"); 
				node != nullptr; node = xmlParser.getNextNode(node, "attribute")) 
		{
			std::string name = xmlParser.getValueByString(node, "name", "");
			std::string type = xmlParser.getValueByString(node, "type", "");
			CHECK_RETURN(name.length() > 0, false, "miss attribute `name`");
			CHECK_RETURN(type.length() > 0, false, "miss attribute `type`");

			auto i = types.find(type.c_str());
			CHECK_RETURN(i != types.end(), false, "illegal type: %s", type.c_str());

			bool rc = descriptor.insert(std::make_pair(name, i->second)).second;
			CHECK_RETURN(rc, false, "duplicate attribute `name`: %s", name.c_str());
		}

		xmlParser.final();
		return true;
	}

	const std::unordered_map<std::string, Entity::value_type>& EntityDescriptor::getDescriptor(u32 tableid) {
		if (!ContainsKey(this->_tables, tableid)) {
			Error << "not found descriptor: " << tableid;
		}
		return this->_tables[tableid];
	}

	INITIALIZE_INSTANCE(EntityDescriptor);
}
