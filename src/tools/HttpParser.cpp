/*
 * \file: HttpParser.cpp
 * \brief: Created by hushouguo at 22:09:27 Sep 30 2018
 */

#include "bundle.h"

// reference: https://github.com/nodejs/http-parser
BEGIN_NAMESPACE_BUNDLE {
	enum PARSER_STATE {
		s_request_start	= 1,

		s_request_method,
		s_request_url,
		s_request_version,
		s_request_line_done,

		s_request_field_name,
		s_request_field_value,
		s_request_header_done,
	};

	/* Macros for character classes; depends on strict-mode  */
#define CR                  '\r'
#define LF                  '\n'
#define SP					' '

	size_t HttpParser::parse(const char* data, size_t len) {
		if (len == 0) {
			return 0;	// expect more data
		}

		this->clear();

		size_t nread = 0;
		PARSER_STATE ps = s_request_method;
		char prevc, c = 0;

		// request line
		size_t prevread = nread;
		while (nread < len && ps != s_request_line_done) {
			prevc = c, c = data[nread++];
			switch (ps) {
				case s_request_method:
					if (c == SP) {
						ps = s_request_url;
						this->_req_method.assign(&data[prevread], nread - prevread - 1);
						prevread = nread;
					}
					break;

				case s_request_url:
					if (c == SP) {
						ps = s_request_version;
						this->_req_url.assign(&data[prevread], nread - prevread - 1);
						prevread = nread;
					}
					break;

				case s_request_version:
					if (c == LF && prevc == CR) {
						ps = s_request_line_done;
						this->_req_version.assign(&data[prevread], nread - prevread - 2);	// erase CR
					}
					break;

				default: CHECK_RETURN(false, -1, "illegal parser state: %d\n", ps);
			}
		}

		CHECK_RETURN(ps == s_request_line_done, 0, "incomplete request line: %s, len: %ld", data, len); // not enough data

		// headers
		prevread = nread;
		std::string name, value;
		ps = s_request_field_name;
		while (nread < len && ps != s_request_header_done) {
			prevc = c, c = data[nread++];
			switch (ps) {
				case s_request_field_name:
					if (c == LF && prevc == CR) {
						//
						//	UNHANDLE INVALID HEADER
						//	"GET / HTTP/1.1\r\n"    
						//	"name\r\n"
						//	"\r\n";
						//
						ps = s_request_header_done;
						break;
					}
					if (c == ':') {
						ps = s_request_field_value;
						name.assign(&data[prevread], nread - prevread - 1);
						prevread = nread;
					}
					break;

				case s_request_field_value:
					if (c == LF && prevc == CR) {
						ps = s_request_field_name;
						value.assign(&data[prevread], nread - prevread - 2);	// erase CR
						prevread = nread;
						if (!value.empty()) {	// erase LWS: linear whitespace
							value.erase(0, value.find_first_not_of(" "));   // ltrim
							value.erase(value.find_last_not_of(" ") + 1);   // rtrim
						}
						bool rc = this->_req_headers.insert(std::make_pair(name, value)).second;
						CHECK_RETURN(rc, -1, "duplicate header: %s, %s\n", name.c_str(), value.c_str());
					}
					break;

				default: CHECK_RETURN(false, -1, "illegal parser state: %d\n", ps);
			}
		}

		CHECK_RETURN(ps == s_request_header_done, 0, "incomplete headers: %s, len: %ld", data, len); // not enough data

		const char* length_value = this->header("Content-Length");
		if (length_value) {
			try {
				size_t content_length = std::stol(length_value);
				nread += content_length;
				if (len < nread) {
					return 0;	// not enough data
				}
			}
			catch(...) {
				CHECK_RETURN(false, -1, "illegal Content-Length: %s", length_value);
			}
		}
		return nread;
	}

	void HttpParser::dump() {
		Trace << "Method:" << this->_req_method << ", " << this->_req_method.length();
		Trace << "Url:" << this->_req_url << ", " << this->_req_url.length();
		Trace << "Version:" << this->_req_version << ", " << this->_req_version.length();
		Trace << "Headers:";
		for (auto& i : this->_req_headers) {
			Trace << "    Name:" << i.first << ", Value:" << i.second << ", " << i.second.length();
		}
	}

	void HttpParser::clear() {
		this->_req_method.clear();
		this->_req_url.clear();
		this->_req_version.clear();
		this->_req_headers.clear();
	}

	const char* HttpParser::header(const char* name) {
		auto i = this->_req_headers.find(name);
		return i != this->_req_headers.end() ? i->second.c_str() : nullptr;
	}
}


