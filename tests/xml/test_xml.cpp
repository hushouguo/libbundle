/*
 * \file: test_xml.cpp
 * \brief: Created by hushouguo at 19:38:12 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;

void test_xml(const char* filename) {
	XmlParser xmlParser;
	if (!xmlParser.open(filename)) {
		Error << "open filename: " << filename << " failure";
		return;
	}

	Registry reg;
	xmlParser.makeRegistry(&reg);
	xmlParser.final();
	reg.dump();
	
	System << "test xml OK";
}

void test_xml2(const char* filename) {
	rapidxml::xml_document<> doc;

	try {
		rapidxml::file<> fileDoc(filename);
		doc.parse<0>(fileDoc.data());
	} catch (std::exception& e) {
		Error.cout("xmlParser exception: %s, filename: %s", e.what(), filename);
		return;
	}

	rapidxml::xml_node<>* root = doc.first_node();
	if (!root) {
		return;
	}

	//fprintf(stderr, "root: %s\n", root->name());
	std::string s = root->name();
	Trace << "root: " << s;

	rapidxml::xml_node<>* node = nullptr;
	for (node = root->first_node(); node != nullptr; node = node->next_sibling()) {
		fprintf(stderr, "node: %s\n", node->name());
		rapidxml::xml_attribute<> * attr = node->first_attribute();
		for (attr = node->first_attribute(); attr != nullptr; attr = attr->next_attribute()) {
			fprintf(stderr, "shard: %s\n", attr->value());
		}
//		MYSQL handler;
//		mysql_init(&handler);
	}


#if 0
	for (XmlParser::XML_NODE node = xmlParser.getChildNode(root, "mysql"); node != nullptr; node = xmlParser.getNextNode(node, "mysql")) {
		u32 shard = xmlParser.getValueByInteger(node, "shard", 0);
		std::string conf = xmlParser.getValueByString(node, "conf", "");
		Trace << "load shard: " << shard << ", conf: " << conf;
		MYSQL handler;
		mysql_init(&handler);
//		MySQL* handler = new MySQL();
//		(void)(handler);
	}
	xmlParser.final();
#endif	

//	char* s = (char*)"hello";
	std::ostream o(nullptr);
	std::stringbuf* buf = new std::stringbuf();
//	std::ostringstream os;
	o.rdbuf(buf);
	o << s;
	fprintf(stderr, "s:%s\n", buf->str().c_str());
}
