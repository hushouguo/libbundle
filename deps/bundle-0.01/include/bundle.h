/*
 * \file: bundle.h
 * \brief: Created by hushouguo at 10:29:59 Jul 06 2018
 */
 
#ifndef __BUNDLE_H__
#define __BUNDLE_H__

#define BUNDLE_VERSION_MAJOR		0
#define BUNDLE_VERSION_MINOR		1
#define BUNDLE_VERSION_PATCH		0

#ifndef PLATFORM_WINDOWS
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS	1
#endif
#endif

#ifndef PLATFORM_LINUX
#if defined(LINUX)
#define PLATFORM_LINUX		1
#endif
#endif

#if !defined(X64) && !defined(X32)
#if defined(_WIN64) || defined(_X64)
#define X64					1
#else
#define X32					1
#endif
#endif

#ifndef DEBUG
#if defined(_DEBUG)
#define DEBUG				1
#endif
#endif

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX)
#error "MISS PLATFORM"
#endif

#if defined(__plusplus)
#if __cplusplus <= 199711L
#error "REQUIRE C++ 11 SUPPORT"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>

#ifdef PLATFORM_LINUX
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <pwd.h>
#include <execinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <getopt.h>
#include <wchar.h>
#include <locale.h>
#include <libgen.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <process.h>
#include <winsock2.h>
#endif

#if defined(__cplusplus)
#include <cassert>
#include <cerrno>
#include <istream>
#include <random>
#include <iostream>
#include <iomanip>
#include <exception>
#include <fstream>  
#include <sstream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <locale>
#include <limits>
#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <chrono>
#include <algorithm>
#include <utility>
#include <vector>
#include <queue>
#include <array>
#include <list>
#include <string>
#include <cstring>
#include <csignal>
#include <map>
#include <stack>
#include <unordered_map>
#if __GNUC__ >= 5
#include <codecvt>
#endif
#endif

// google protobuf-3.6.1

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/message.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

//NOTE: for now, 0mq version is 4.3.0, ZMQ_SERVER/ZMQ_CLIENT still is draft api
#define ZMQ_BUILD_DRAFT_API
#include "zmq.h"

// mysql 5.6.37
//#include <mysql/mysql.h>
//#include <mysql/errmsg.h>
#include "mysql.h"
#include "errmsg.h"

// libevent
#include "event.h"
#include "evhttp.h"

// libcurl-7.58.0
#include <curl/curl.h>

// tcmalloc-2.6.1
#include "gperftools/tcmalloc.h"

// rapidxml-1.13
#include "rapidxml-1.13/rapidxml.hpp"  
#include "rapidxml-1.13/rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml-1.13/rapidxml_print.hpp"//rapidxml::print 

// rapidjson-1.1.0
#include "rapidjson-1.1.0/document.h"

// libuv-1.23.2
#include "uv.h"

using u8 		= 						uint8_t;
using u16 		= 						uint16_t;
using u32		= 						uint32_t;
using u64		= 						uint64_t;
using s8		= 						int8_t;
using s16		= 						int16_t;
using s32		= 						int32_t;
using s64		= 						int64_t;

#ifdef PLATFORM_LINUX
using Byte		= 						unsigned char;
#endif

#ifdef PLATFORM_WINDOWS
#define PATH_MAX						MAX_PATH
#define TEMP_FAILURE_RETRY(EXEC)		EXEC
#define MSG_DONTWAIT					0
#define MSG_NOSIGNAL					0
using ssize_t	=						int;
#endif

#ifdef PLATFORM_WINDOWS
#define SOCKET							intptr_t
#else
#define SOCKET							int
#endif

//
// return value:
//	-1: error happen
//	 0: incomplete package
// > 0: package length
//
using MESSAGE_SPLITER	=				std::function<int(const void*, size_t)>;

#if !defined(__cplusplus)
using bool		= 						int;
#define true							1
#define false							0
#define nullptr							NULL
#define min(a,b) 						(((a) > (b)) ? (b) : (a))
#define max(a,b) 						(((a) > (b)) ? (a) : (b))
#endif

#ifndef offsetof
#define offsetof(STRUCTURE,FIELD)		((size_t)((char*)&((STRUCTURE*)0)->FIELD))
#endif

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define	KB								1024U
#define MB								1048576U
#define GB								1073741824U
#define TB								1099511627776ULL

#define MINUTE							60U
#define HOUR							3600U
#define DAY								86400U

#define SafeFree(P)						do { if(P) { ::free((void*)P); (P) = nullptr; } } while(0)
#define SafeDelete(P)					do { if(P) { delete (P); (P) = nullptr; } } while(0)
#define SafeClose(S)					do { if(S > 0) { ::close(S); S = -1; } } while(0)

#define BEGIN_NAMESPACE_BUNDLE			namespace bundle
BEGIN_NAMESPACE_BUNDLE {
}

#include "tools/Callback.h"
#include "tools/Constructor.h"
#include "tools/Noncopyable.h"
#include "tools/Singleton.h"
#include "tools/ByteBuffer.h"
#include "tools/Registry.h"
#include "tools/Tools.h"
#include "tools/Spinlocker.h"
#include "tools/LockfreeQueue.h"
#include "tools/HttpParser.h"
#include "entry/Entry.h"
#include "entry/Manager.h"
#include "sha/sha1.h"
#include "base64/base64.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "xml/XmlParser.h"
#include "easylog/Easylog.h"
#define CSV_IO_NO_THREAD 
#include "csv/CsvReader.h"
#include "csv/CsvParser.h"
#include "config/Config.h"
#include "socket/Socketmessage.h"
#include "socket/SocketServer.h"
#include "socket/SocketClient.h"
#include "socket/WebSocketServer.h"
#include "socket/WebServer.h"
#include "socket/CgiServer.h"
#include "net/Netmessage.h"
#include "net/NetworkInterface.h"
#include "net/NetworkService.h"
#include "net/NetworkClient.h"
#include "net/MessageQueue.h"
#include "net/MessageQueueClient.h"
#include "net/MessageQueueService.h"
#include "db/MySQLStatement.h"
#include "db/MySQLResult.h"
#include "db/MySQL.h"
#include "entity/Entity.h"
#include "entity/EntityDescriptor.h"
#include "recordclient/RecordClient.h"

#endif
