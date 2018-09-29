/*
 * \file: Poll.h
 * \brief: Created by hushouguo at 14:39:25 Jun 28 2018
 */
 
#ifndef __POLL_H__
#define __POLL_H__

#define NM_POLL_EVENT		128

BEGIN_NAMESPACE_BUNDLE {
	class Socket;
	class WorkerProcess;
	class Poll {
		public:
			Poll(WorkerProcess* slotWorker);
			~Poll();

		public:
			void run(int milliseconds);
			
		public:
			bool addSocket(SOCKET s);
			bool removeSocket(SOCKET s);
			bool setSocketPollout(SOCKET s, bool value);

		private:
			int _epfd = -1;
			WorkerProcess* _slotWorker = nullptr;
			struct epoll_event _events[NM_POLL_EVENT];
	};
}

#endif
