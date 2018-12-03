/*
 * \file: test_uv.cpp
 * \brief: Created by hushouguo at 13:57:56 Oct 25 2018
 */

#include "bundle.h"

using namespace bundle;


void wait_for_a_while(uv_idle_t* handle) {
	static int counter = 0;
    counter++;
    if (counter >= 10e6) {
		Trace << "stop idle";
		printf("handle->data: %s\n", (const char*) uv_handle_get_data((uv_handle_t*)handle));
        uv_idle_stop(handle);
	}
}

void dump_handle(uv_handle_t* handle) {
	Trace << "dump handle";
	Trace << "  type: " << uv_handle_get_type(handle);
	Trace << "  type name: " << uv_handle_type_name(uv_handle_get_type(handle));
	Trace << "  size: " << uv_handle_size(uv_handle_get_type(handle));
	Trace << "  data: " << uv_handle_get_data(handle);
}

void test_uv_idle() {
    //uv_idle_t idler;
	//const char* s = "this is data";

    //uv_idle_init(uv_default_loop(), &idler);
	//uv_handle_set_data((uv_handle_t*)&idler, (void*)s);
	//dump_handle((uv_handle_t*) &idler);
    //uv_idle_start(&idler, wait_for_a_while);


    //printf("Idling...\n");
	Trace << "idleing ... ";
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	Trace << "uv loop over";
    uv_loop_close(uv_default_loop());
}

void test_uv_interface_address() {
	uv_interface_address_t* address = nullptr;
	int count = 0;
	uv_interface_addresses(&address, &count);
	Trace << "interface list: ";
	for (int i = 0; i < count; ++i) {
		char buffer[32];
		uv_interface_address_t interface = address[i];
		Trace << "interface: " << i;
		Trace << "  Name: " << interface.name;
		Trace << "  phys_addr: " << interface.phys_addr;
		Trace << "  is_internal: " << interface.is_internal;
		if (interface.address.address4.sin_family == AF_INET) {
			uv_ip4_name(&interface.address.address4, buffer, sizeof(buffer));
			Trace << "  address4: " << buffer;
			uv_ip4_name(&interface.netmask.netmask4, buffer, sizeof(buffer));
			Trace << "  mask4: " << buffer;

		}
		else if (interface.address.address4.sin_family == AF_INET6) {
			uv_ip6_name(&interface.address.address6, buffer, sizeof(buffer));
			Trace << "  address6: " << buffer;
			uv_ip6_name(&interface.netmask.netmask6, buffer, sizeof(buffer));
			Trace << "  mask6: " << buffer;
		}
	}
	uv_free_interface_addresses(address, count);
}

uv_tcp_t server_handle;
void on_alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	Trace << "alloc_db, suggested_size: " << suggested_size << ", buf: " << buf;
}

void on_read_callback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
	Trace << "read_cb, nread: " << nread << ", buf: " << buf << ", stream: " << stream;
}

void on_new_connection(uv_stream_t* server, int status) {
	Trace << "on_new_connection: " << status;
	if (status < 0) {
		Error << "on_new_connection error: " << uv_strerror(status);
		return;
	}

	uv_tcp_t* client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client);
	if (uv_accept(server, (uv_stream_t*) client) == 0) {
		Trace << "read start: " << client;
		uv_read_start((uv_stream_t*) client, on_alloc_callback, on_read_callback);
	}
	else {
		uv_close((uv_handle_t*) client, nullptr);
	}
}

void test_uv_start_server(const char* address, int port) {
#if 0	
	uv_tcp_init(uv_default_loop(), &server_handle);
	struct sockaddr addr;
	uv_ip4_addr(address, port, &addr);
	uv_tcp_bind(&server_handle, &addr, 0);
	int rc = uv_listen((uv_stream_t*) &server_handle, 128, on_new_connection);
	if (rc) {
		Error << "uv_listen error: " << uv_strerror(rc);
		return;
	}
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
#endif	
}

void test_uv() {
	//test_uv_idle();
	//test_uv_interface_address();
	test_uv_start_server("0.0.0.0", 12306);
}

