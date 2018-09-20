/*
 * \file: NetworkService.cpp
 * \brief: Created by hushouguo at 14:42:00 Aug 30 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	class NetworkTask : public NetworkInterface {
		public:
			virtual ~NetworkTask() {}
			NetworkTask(NetworkService* ns, SOCKET s) : _ns(ns), _socket(s) {}

		public:
			SOCKET fd() override { return this->_socket; }
			void sendMessage(const Netmessage* netmsg) override {
				this->_ns->socketServer()->sendMessage(this->_socket, netmsg, netmsg->len);
			}
			void sendMessage(s32 msgid, const google::protobuf::Message* msg) override {
				int dataSize = msg->ByteSize();
				char buffer[sizeof(Netmessage) + dataSize];
				Netmessage* netmsg = (Netmessage *) buffer;
				netmsg->len = sizeof(Netmessage) + dataSize;
				netmsg->id = msgid;
				netmsg->flags = 0;
				netmsg->timestamp = 0;
				bool rc = msg->SerializeToArray(netmsg->payload, dataSize);
				CHECK_RETURN(rc, void(0), "SerializeToArray error");
				this->_ns->socketServer()->sendMessage(this->_socket, netmsg, netmsg->len);
			}
			void sendMessage(const void* payload, size_t payload_len) override {
				this->_ns->socketServer()->sendMessage(this->_socket, payload, payload_len);
			}

		private:
			NetworkService* _ns = nullptr;
			SOCKET _socket = -1;
	};

	NetworkInterface* NetworkService::getNetworkInterface(SOCKET s) {
		return FindOrNull(this->_tasks, s);
	}

	NetworkTask* NetworkService::spawnConnection(SOCKET s) {
		NetworkTask* task = new NetworkTask(this, s);
		bool rc = this->_tasks.insert(std::make_pair(s, task)).second;
		if (!rc) {
			SafeDelete(task);
			CHECK_RETURN(false, nullptr, "duplicate NetworkTask: %d", s);
		}
		return task;
	}

	void NetworkService::closeConnection(SOCKET s) {
		auto i = this->_tasks.find(s);
		CHECK_RETURN(i != this->_tasks.end(), void(0), "not exist NetworkTask: %d", s);
		this->_tasks.erase(i);
	}

	bool NetworkService::start(const char* address, int port) {
		SafeDelete(this->_socketServer);
		this->_socketServer = SocketServerCreator::create([](const Byte* buffer, size_t len) -> int {
				Netmessage* netmsg = (Netmessage*) buffer;
				return len < sizeof(Netmessage) || len < netmsg->len ? 0 : netmsg->len;
				});

		bool rc = this->_socketServer->start(address, port);
		if (!rc) {
			SafeDelete(this->_socketServer);
			return false;
		}

		size_t maxsize = sConfig.get("service.security.maxsize", 0u);
		if (maxsize > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_MAXSIZE, &maxsize, sizeof(maxsize));
		}

		size_t silence_second = sConfig.get("service.security.silence_second", 0u);
		if (silence_second > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_SILENCE_SECOND, &silence_second, sizeof(silence_second));
		}

		size_t threshold_interval = sConfig.get("service.security.threshold_interval", 0u);
		if (threshold_interval > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_THRESHOLD_INTERVAL, &threshold_interval, sizeof(threshold_interval));
		}

		size_t threshold_message = sConfig.get("service.security.threshold_message", 0u);
		if (threshold_message > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_THRESHOLD_MESSAGE, &threshold_message, sizeof(threshold_message));
		}

		setup_runtime_environment();
		Trace.cout("NetworkService listening on %s:%d", address, port);
		return true;
	}

	bool NetworkService::update() {
		CHECK_RETURN(this->_socketServer, false, "please call `start` to init service");
		while (!this->isstop()) {
			SOCKET s = -1;
			bool is_establish = false, is_close = false;
			bool rc = true;
			const Socketmessage* msg = this->_socketServer->receiveMessage(s, is_establish, is_close);
			if (msg) {
				if (is_establish) {
					NetworkTask* task = this->spawnConnection(s);
					if (this->_establishConnection) {
						this->_establishConnection(this, task);
					}
				}
				else if (is_close) {
					NetworkTask* task = FindOrNull(this->_tasks, s);
					if (task) {
						if (this->_lostConnection) {
							this->_lostConnection(this, task);
						}
						this->closeConnection(s);
					}
					else {
						Error << "not found NetworkTask: " << s;
					}
				}
				else {
					NetworkTask* task = FindOrNull(this->_tasks, s);
					if (task && this->_msgParser) {
						rc = this->_msgParser(this, task, (const Netmessage *) msg->payload);
					}
				}
				if (rc) {
					this->socketServer()->releaseMessage(msg);
				}
			}
			else {
				return true;
			}
		}
		return !this->isstop();
	}

	void NetworkService::releaseMessage(const Netmessage* netmsg) {
		Socketmessage* msg = (Socketmessage *) ((Byte*) netmsg - offsetof(Socketmessage, payload));
		assert(msg->payload_len == netmsg->len);
		assert(this->socketServer());
		this->socketServer()->releaseMessage(msg);
	}

	void NetworkService::close(NetworkInterface* task) {
		if (this->_socketServer) {
			this->_socketServer->close(task->fd());
		}
	}

	void NetworkService::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			for (auto& i : this->_tasks) {
				NetworkTask* task = i.second;
				SafeDelete(task);
			}
			this->_tasks.clear();
			if (this->_socketServer) {
				this->_socketServer->stop();
			}
		}
	}

	void NetworkService::setup_runtime_environment() {
		this->init_signal();
		size_t stack_size = sConfig.get("limit.stack_size", 0u);
		if (stack_size > 0) {
			setStackSizeLimit(stack_size);
		}

		size_t max_files = sConfig.get("limit.max_files", 0u);
		if (max_files > 0) {
			setOpenFilesLimit(max_files);
		}

		Trace << "StackSize: " << getStackSizeLimit() << ", MaxOpenFiles: " << getOpenFilesLimit();

		u32 shard = sConfig.get("shard.id", 0u);
		if (shard > 0) {
			Trace << "shard: " << shard;
		}
		else {
			Trace << "shard: not configured";
		}

		Trace << "all 3rd libraries:";
		Trace.cout("    libbundle: %d.%d.%d", BUNDLE_VERSION_MAJOR, BUNDLE_VERSION_MINOR, BUNDLE_VERSION_PATCH);

#ifdef TC_VERSION_MAJOR		
		Trace.cout("    tcmalloc: %d.%d%s", TC_VERSION_MAJOR, TC_VERSION_MINOR, TC_VERSION_PATCH);
#else
		Trace.cout("    not link tcmalloc");
#endif

#ifdef LIBEVENT_VERSION
		Trace.cout("    libevent: %s", LIBEVENT_VERSION);
#endif

#ifdef ZMQ_VERSION_MAJOR
		Trace.cout("    libzmq: %d.%d.%d", ZMQ_VERSION_MAJOR, ZMQ_VERSION_MINOR, ZMQ_VERSION_PATCH);
#endif

#ifdef LUAJIT_VERSION
		Trace.cout("    luaJIT: %s -- %s", LUAJIT_VERSION, LUAJIT_COPYRIGHT);
#endif

#ifdef GOOGLE_PROTOBUF_VERSION
		Trace.cout("    protobuf: %d, library: %d", GOOGLE_PROTOBUF_VERSION, GOOGLE_PROTOBUF_MIN_LIBRARY_VERSION);
#endif

		Trace.cout("    rapidxml: 1.13");

#ifdef MYSQL_SERVER_VERSION		
		Trace.cout("    mysql: %s", MYSQL_SERVER_VERSION);
#endif

		Trace.cout("    gcc version: %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	}

	void NetworkService::init_signal() {
		struct sigaction act;
		/*signal(SIGHUP, SIG_IGN);*/
		signal(SIGPIPE, SIG_IGN);
		/* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
		 * Otherwise, sa_handler is used. */
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		act.sa_handler = [](int sig) {
			Alarm << "receive signal: " << sig;
			switch (sig) {
				case SIGHUP:
					// reload configure
					break;

				case SIGINT:
				case SIGTERM:
				case SIGQUIT:				
					sConfig.syshalt();	
					Alarm << "system is being shutdown ..."; 
					break;

				case SIGUSR1:
#ifdef TC_VERSION_MAJOR				
					if (true) {
						struct mallinfo info = tc_mallinfo();
						System << "total allocated space: " << info.uordblks << " bytes";
						if (!sConfig.runasdaemon) {
							tc_malloc_stats();
						}
					}
#endif

				default: break;
			}
		};
		sigaction(SIGHUP, &act, nullptr);
		sigaction(SIGINT, &act, nullptr);
		sigaction(SIGTERM, &act, nullptr);
		sigaction(SIGQUIT, &act, nullptr);
		sigaction(SIGUSR1, &act, nullptr);
		sigaction(SIGUSR2, &act, nullptr);
		sigaction(SIGILL, &act, nullptr);		
	}

	NetworkService::~NetworkService() {
		this->stop();
	}
}
