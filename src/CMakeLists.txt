set(target bundle) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(LIBRARY_OUTPUT_PATH ${BUNDLE_DIR}/lib)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(${ROOT_DIR})
include_directories(${DEPS_DIR})
include_directories(${BUNDLE_DIR}/include)
include_directories(${PROTOBUF_DIR}/include)
include_directories(${TCMALLOC_DIR}/include)
include_directories(${MYSQL_DIR}/include)
include_directories(${RAPIDXML_DIR})
include_directories(${RAPIDJSON_DIR})
include_directories(${LIBEVENT_DIR}/include)
include_directories(${CURL_DIR}/include)
include_directories(${LIBUV_DIR}/include)

aux_source_directory(base64 src)
aux_source_directory(config src)
aux_source_directory(db src)
aux_source_directory(easylog src)
aux_source_directory(entity src)
aux_source_directory(lua src)
aux_source_directory(net src)
aux_source_directory(recordclient src)
aux_source_directory(sha src)
aux_source_directory(socket src)
aux_source_directory(time src)
aux_source_directory(tools src)
aux_source_directory(xml src)

add_library(${target} ${src})
