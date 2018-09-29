/*
 * \file: WorkerProcess.h
 * \brief: Created by hushouguo at 05:08:54 Sep 22 2018
 */
 
#ifndef __WORKER_PROCESS_H__
#define __WORKER_PROCESS_H__

#define MAX_CONNECTION_FD				65536
#define ASSERT_SOCKET(S)				assert(S >= 0 && S < MAX_CONNECTION_FD)
#define WAKE_WORKER_PROCESS_SIGNAL		SIGRTMIN

BEGIN_NAMESPACE_BUNDLE {
	class WorkerProcess : public Entry<u32> {
		public:
			void readSocket(SOCKET);
			void writeSocket(SOCKET);
			void errorSocket(SOCKET);

			void addSocket(SOCKET, bool is_listening = false);
			void closeSocket(SOCKET);
			void pushMessage(SOCKET, Socketmessage*);
			void pushMessage(SOCKET, const void*, size_t);

			inline size_t totalConnections() { return this->_totalConnections; }
			inline const MESSAGE_SPLITER& splitMessage() { return this->_splitMessage; }
			
		public:
			WorkerProcess(u32 id, MESSAGE_SPLITER splitMessage, LockfreeQueue<Socketmessage*>* recvQueue);
			~WorkerProcess();
			const char* getClassName() override { return "WorkerProcess"; }

		private:
			Poll _poll;
			Socket* _sockets[MAX_CONNECTION_FD];
			SOCKET _maxfd = -1;
			std::thread* _threadWorker = nullptr;
			LockfreeQueue<Socketmessage*> _sendQueue, *_recvQueue = nullptr;
			bool _stop = false;
			inline bool isstop() { return this->_stop; }
			void stop();
			MESSAGE_SPLITER _splitMessage = nullptr;
			size_t _totalConnections = 0;

			void run();
			void pushMessage(Socketmessage*);
			void newSocket(SOCKET, bool is_listening);
			void removeSocket(SOCKET, const char*);
			void sendMessage(SOCKET, Socketmessage*);
			void multisendMessage(Socketmessage*);
			void checkSocket();
			void handleMessage();
			void acceptSocket(Socket*);
			void readSocket(Socket*);
	};
}

#endif
