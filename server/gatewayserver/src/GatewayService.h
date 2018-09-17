/*
 * \file: GatewayService.h
 * \brief: Created by hushouguo at 09:34:23 Sep 01 2018
 */
 
#ifndef __GATEWAYSERVICE_H__
#define __GATEWAYSERVICE_H__

class GatewayService : public NetworkService {
	public:
		GatewayService();

	public:
		bool init();

	private:
		bool msgParser(NetworkInterface* task, const Netmessage* netmsg);
};

#define sGatewayService bundle::Singleton<GatewayService>::getInstance()

#endif
