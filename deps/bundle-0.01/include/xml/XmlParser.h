/*
 * \file: XmlParser.h
 * \brief: Created by hushouguo at 09:53:50 Jan 22 2018
 */
 
#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

BEGIN_NAMESPACE_BUNDLE {
	class XmlParser {
		public:
			XmlParser();
			~XmlParser();

		public:
			typedef rapidxml::xml_node<>* XML_NODE;

			bool open(const char* xmlfile);
			bool open(std::string xmlfile);
			void final();
			XML_NODE getRootNode();
			XML_NODE getChildNode(XML_NODE xmlNode, const char* name = nullptr);
			XML_NODE getNextNode(XML_NODE xmlNode, const char* name = nullptr);
			const char* getValueByString(XML_NODE xmlNode, const char* name, const char* defaultValue = nullptr);
			const uint32_t getValueByInteger(XML_NODE xmlNode, const char* name, uint32_t defaultValue = 0);
			void getValues(XML_NODE xmlNode, std::map<std::string, std::string> values);

		public:
			void dump();
			void makeRegistry(Registry* registry, const char* prefix = "");

		private:
			rapidxml::xml_document<> doc;
			void dump(XML_NODE xmlNode, int depth);
			void makeRegistry(Registry* registry, XML_NODE xmlNode, std::string name);
	};	
}

#endif
