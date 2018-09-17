/*
 * \file: ClientTaskManager.h
 * \brief: Created by hushouguo at 08:05:52 Sep 01 2018
 */
 
#ifndef __CLIENTTASKMANAGER_H__
#define __CLIENTTASKMANAGER_H__

class ClientTaskManager : public Manager<ClientTask> {
};

#define sClientTaskManager bundle::Singleton<ClientTaskManager>::getInstance()

#endif
