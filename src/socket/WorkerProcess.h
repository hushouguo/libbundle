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
			inline bool isstop() { return this->_stop; }
			void stop();

		public:
			WorkerProcess(u32 id, LockfreeQueue<Socketmessage*>* recvQueue);
			~WorkerProcess();

		private:
			Poll _poll;
			Socket* _sockets[MAX_CONNECTION_FD];
			SOCKET _maxfd = BUNDLE_INVALID_SOCKET;
			std::thread* _threadWorker = nullptr;
			LockfreeQueue<Socketmessage*> _sendQueue, *_recvQueue = nullptr;
			LockfreeQueue<SOCKET> _fdsQueue;
			bool _stop = false;

			void proceed();
			void addSocket(Socket*, bool spread = true);
			void removeSocket(SOCKET);
			void sendMessage(SOCKET, Socketmessage*);
			void multisendMessage(Socketmessage*);
			void checkSocket();
			void acceptSocket();
			void handleMessage();
			void acceptSocket(Socket*);
			void readSocket(Socket*);
	};
}

#endif
