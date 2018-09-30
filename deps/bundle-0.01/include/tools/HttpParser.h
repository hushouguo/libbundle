/*
 * \file: HttpParser.h
 * \brief: Created by hushouguo at 22:03:03 Sep 30 2018
 */
 
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

BEGIN_NAMESPACE_BUNDLE {
	class HttpParser {
		public:
			void dump();
			void clear();
			size_t parse(const char* data, size_t len);
			inline const std::string& method() { return this->_req_method; }
			inline const std::string& url() { return this->_req_url; }
			inline const std::string& version() { return this->_req_version; }
			const char* header(const char* name);
			inline std::unordered_map<std::string, std::string>& headers() { return this->_req_headers; }

		private:
			std::string _req_method, _req_url, _req_version;
			std::unordered_map<std::string, std::string> _req_headers;
	};
}

#endif
