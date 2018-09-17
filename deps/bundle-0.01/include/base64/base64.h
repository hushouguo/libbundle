/*
 * \file: base64.h
 * \brief: Created by hushouguo at 10:25:53 Jul 06 2018
 */
 
#ifndef __BASE64_H__
#define __BASE64_H__

BEGIN_NAMESPACE_BUNDLE {
	//
	// base64 encode & decode
	//
	void base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, std::string& ret_string);
	void base64_decode(std::string const& encoded_string, std::string& ret_string);
}

#endif
