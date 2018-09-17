/*
 * \file: CentralClient.h
 * \brief: Created by hushouguo at 08:33:56 Sep 01 2018
 */
 
#ifndef __CENTRALCLIENT_H__
#define __CENTRALCLIENT_H__

class CentralClient : public NetworkClient {
	public:
		CentralClient();

	public:
		bool init();

	private:
		bool msgParser(NetworkInterface* task, const Netmessage* netmsg);
};

#define sCentralClient bundle::Singleton<CentralClient>::getInstance()

#endif
