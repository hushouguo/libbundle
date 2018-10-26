set(target gatewayserver) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH bin)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(${DEPS_DIR})
include_directories(${BUNDLE_DIR}/include)
include_directories(${PROTOBUF_DIR}/include)
include_directories(${TCMALLOC_DIR}/include)
include_directories(${MYSQL_DIR}/include)
include_directories(${RAPIDXML_DIR})
include_directories(${RAPIDJSON_DIR})
include_directories(${LIBEVENT_DIR}/include)
include_directories(${LIBUV_DIR}/include)

aux_source_directory(src src)

link_directories(/usr/lib64) 
link_directories(/usr/lib64/mysql)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(${target} ${src})

target_link_libraries(${target} rt dl crypto curl -pthread)
target_link_libraries(${target} ${BUNDLE_DIR}/lib/libbundle.a)
target_link_libraries(${target} ${PROTOBUF_DIR}/lib/libprotobuf.a)
target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a)

