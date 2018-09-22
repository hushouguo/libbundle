/*
 * \file: WorkerProcess.cpp
 * \brief: Created by hushouguo at 05:42:03 Sep 22 2018
 */

BEGIN_NAMESPACE_BUNDLE {
	WorkerProcess::WorkerProcess(u32 id, LockfreeQueue<Socketmessage*>* recvQueue) 
		: Entry<u32>(id) {
		memset(this->_sockets, 0, sizeof(this->_sockets));
		this->_threadWorker = new std::thread([this]() {
				this->proceed();
				});
		this->_recvQueue = recvQueue;
	}

	WorkerProcess::~WorkerProcess() {
		this->stop();
	}

	void WorkerProcess::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_threadWorker) {
				pthread_kill(this->_threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
				if (this->_threadWorker->joinable()) {
					this->_threadWorker->join();
				}
			}

			// destroy worker thread
			SafeDelete(this->_threadWorker);
		
			// close connfd
			while (!this->_fdsQueue.empty()) {
				SOCKET s = this->_fdsQueue.pop_front();
				::close(s);
			}

			// release Socket object
			for (auto& so : this->_sockets) {
				SafeDelete(so);
			}

			// clean sendQueue
			while (!this->_sendQueue.empty()) {
				const Socketmessage* msg = this->_sendQueue.pop_front();
				assert(msg);
				assert(msg->magic == MAGIC);
				bundle::releaseMessage(msg);
			}
		}
	}

	void WorkerProcess::proceed() {
		setSignal(WAKE_WORKER_PROCESS_SIGNAL);
		while (!this->isstop()) {
			this->checkSocket();
			this->acceptSocket();
			this->handleMessage();
			this->poll.run(-1, this->readSocket, this->writeSocket, this->errorSocket);
		}
		Debug << "WorkerProcess: " << this->id << " thread exit, maxfd: " << this->_maxfd 
			<< ", writeQueue: " << this->_writeQueue.size() << ", fdsQueue: " << this->_fdsQueue.size();
	}

	void WorkerProcess::addSocket(SOCKET newfd, bool is_listening) {
		ASSERT_SOCKET(newfd);
		Socket* so = new Socket(newfd, this->_splitMessage);//TODO:
		so->set_listening(is_listening);
		this->addSocket(so, false);
	}

	void WorkProcess::addSocket(Socket* so, bool spread) {
		assert(so);
		ASSERT_SOCKET(so->fd());
		assert(this->_sockets[so->fd()] == nullptr);
		this->_sockets[so->fd()] = so;
		this->_poll.addSocket(so->fd());
		if (so->fd() > this->_maxfd) { this->_maxfd = so->fd(); }i
		if (spread) {
			Socketmessage* msg = allocateMessage(so->fd(), SM_OPCODE_ESTABLISH);
			//
			// throw establish message
			//
			this->_recvQueue->push_back(msg);
		}
		Debug << "establish connection: " << so->fd() << ", slot: " << this->id;
	}

	void WorkerProcess::removeSocket(SOCKET s) {
		ASSERT_SOCKET(s);
		Socket* so = this->_sockets[s];
		//assert(so != nullptr);
		CHECK_ALARM(so != nullptr, "socket: %d not exist", s);
		this->_poll.removeSocket(s);
		this->_sockets[s] = nullptr;
		SafeDelete(so);
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
		//
		// throw close message
		//
		this->_recvQueue.push_back(msg);
		Debug << "lost connection: " << s << ", slot: " << this->id;
	}

	void WorkerProcess::sendMessage(SOCKET s, Socketmessage* msg) {
		ASSERT_SOCKET(s);
		Socket* so = this->_sockets[s];
		if (so && !so->sendMessage(msg)) {
			//
			// send message to socket error
			//
			this->removeSocket(s);
		}
	}

	void WorkerProcess::multisendMessage(Socketmessage* msg) {
		assert(msg);
		assert(msg->payload_len > 0);
		for (SOCKET s = 0; s <= this->_maxfd; ++s) {
			ASSERT_SOCKET(s);
			Socket* so = this->_sockets[s];
			if (so) {
				Socketmessage* newmsg = allocateMessage(s, msg->opcode, msg->payload, msg->payload_len);
				this->sendMessage(s, newmsg);
			}
		}
		//
		// release origin msg
		//
		bundle::releaseMessage(msg);
	}

	void WorkerProcess::checkSocket() {
	}

	void WorkerProcess::acceptSocket() {
		while (!this->_fdsQueue.empty()) {
			SOCKET newfd = this->_fdsQueue.pop_front();
			ASSERT_SOCKET(newfd);
			Socket* so = new Socket(newfd, this->_splitMessage);//TODO:
			so->set_listening(false);
			this->addSocket(so, true);
		}
	}

	void WorkerProcess::handleMessage() {
		while (!this->_sendQueue.empty()) {
			Socketmessage* msg = slot->_sendQueue.pop_front();
			assert(msg);
			assert(msg->magic == MAGIC);
			if (msg->s == BUNDLE_BROADCAST_SOCKET) {
				this->multisendMessage(msg);
			}
			else {
				ASSERT_SOCKET(msg->s);
				switch (msg->opcode) {
					case SM_OPCODE_CLOSE: 
						this->removeSocket(msg->s);
						bundle::releaseMessage(msg);
						break;

					case SM_OPCODE_MESSAGE: 
						if (true) {
							Socket* so = this->_sockets[msg->s];
							if (!so) {
								//
								// socket maybe already be closed
								//
								bundle::releaseMessage(msg);
							}
							else {
								this->sendMessage(msg->s, msg); 
							}
						}
						break;

					case SM_OPCODE_ESTABLISH:
						this->addSocket(msg->s);
						bundle::releaseMessage(msg);
						break;

					default: assert(false); break;
				}					
			}
		}
	}

	void WorkerProcess::readSocket(SOCKET s) {
		ASSERT_SOCKET(s);
		Socket* so = this->_sockets[s];
		assert(so != nullptr);
		if (so->is_listening()) {
			this->acceptSocket(so);	
		}
		else {
			this->readSocket(so);
		}
	}
	
	void WorkerProcess::acceptSocket(Socket* so) {
		assert(so);
		assert(so->is_listening());
		while (!this->isstop()) {
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);
			SOCKET newfd = ::accept(so->fd(), (struct sockaddr*)&addr, &len);
			if (newfd < 0) {
				if (interrupted()) {
					continue;
				}
				if (wouldblock()) {
					break; // no more connection
				}
				CHECK_RETURN(false, void(0), "accept error:%d,%s", errno, strerror(errno));
			}
			ASSERT_SOCKET(newfd);
			WorkerProcess* workerProcess = this->getWorkerProcess(newfd); // TODO:

			//TODO:
			workerProcess->addSocket(newfd, false);

			SlotProcess* slot_worker = this->getWorkerProcess(newfd);
			if (slot_worker == slot) {
				slot_worker->fdslist.push_back(newfd);
			}
			else {
				slot_worker->fdslocker.lock();
				slot_worker->fdslist.push_back(newfd);
				slot_worker->fdslocker.unlock();
				if (slot_worker->threadWorker) {
					pthread_kill(slot_worker->threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
				}
			}
		}
	}

	void WorkerProcess::readSocket(Socket* so) {
		assert(so);
		assert(!so->is_listening());
		while (!this->isstop()) {
			Socketmessage* newmsg = nullptr;
			if (!so->receiveMessage(newmsg)) {
				//
				// read socket error
				//
				this->removeSocket(so->fd());
				return;
			}
			if (newmsg) {
				//
				// get newmsg from Socket
				//
				this->_recvQueue.push_back(newmsg);
				Debug << "newmsg from slot: " << this->id;
			}
			else {
				return; }
		}			
	}

	void WorkerProcess::writeSocket(SOCKET s) {
		ASSERT_SOCKET(s);
		Socket* so = this->_sockets[s];
		assert(so != nullptr);
		//assert(!so->is_listening());
		CHECK_RETURN(!so->is_listening(), void(0), "socket: %d is listening", s);
		if (!so->is_listening() && !so->sendMessage()) {
			//
			// write socket error
			//
			thist->removeSocket(s);
		}
	}

	void WorkerProcess::errorSocket(SOCKET s) {
		ASSERT_SOCKET(s);
		Socket* so = this->_sockets[s];
		assert(so != nullptr);
		this->removeSocket(s);
	}
}
