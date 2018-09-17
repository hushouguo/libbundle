/*
 * \file: ClientTask.h
 * \brief: Created by hushouguo at 08:02:40 Sep 01 2018
 */
 
#ifndef __CLIENTTASK_H__
#define __CLIENTTASK_H__

class ClientTask : public Entry<SOCKET> {
	public:
		ClientTask(NetworkInterface* networkInterface) 
			: Entry<SOCKET>(networkInterface->fd())
			, _networkInterface(networkInterface) {}
		const char* getClassName() override { return "ClientTask"; }
	
	public:
		inline NetworkInterface* networkInterface() { return this->_networkInterface; }

	private:
		NetworkInterface* _networkInterface = nullptr;
};

#endif
