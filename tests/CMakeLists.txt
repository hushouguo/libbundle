set(target tests) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH .)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(${DEPS_DIR})
include_directories(${BUNDLE_DIR}/include)
include_directories(${PROTOBUF_DIR}/include)
include_directories(${TCMALLOC_DIR}/include)
include_directories(${MYSQL_DIR}/include)
include_directories(${RAPIDXML_DIR})
include_directories(${RAPIDJSON_DIR})
include_directories(${LIBUV_DIR}/include)
include_directories(${LIBZMQ_DIR}/include)

aux_source_directory(easylog src)
aux_source_directory(tools src)
aux_source_directory(net src)
aux_source_directory(csv src)
aux_source_directory(xml src)
aux_source_directory(recordclient src)
aux_source_directory(entity src)
aux_source_directory(lockfree src)
aux_source_directory(uv src)
aux_source_directory(zmq src)

link_directories(/usr/lib64) 
link_directories(/usr/lib64/mysql)

add_executable(${target} ${src} main.cpp)

target_link_libraries(${target} rt dl curl crypto unwind -pthread)
target_link_libraries(${target} ${BUNDLE_DIR}/lib/libbundle.a)
target_link_libraries(${target} ${PROTOBUF_DIR}/lib/libprotobuf.a)
target_link_libraries(${target} ${LIBEVENT_DIR}/lib/libevent.a)
target_link_libraries(${target} ${LIBUV_DIR}/lib/libuv.a)
target_link_libraries(${target} ${LIBZMQ_DIR}/lib/libzmq.a)
target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a)

