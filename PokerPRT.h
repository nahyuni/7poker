#ifndef _Poker_PRT_H_
#define _Poker_PRT_H_

#include "MLib.h"
#include "MLibThread.h"
#include "MLibMutex.h"
#include "MLibHeaderPacket.h"
#include "MLibEpoll.h"

#include <list>
#include <deque>


extern MLibMutex m_userListMutex;
extern MLibMutex m_queueListMutex;
extern deque<MLibHeaderPacket> m_queuePacket;

#ifndef S_MYSQL
#define S_MySql MLibSingleton<MySqlDB>::getInstance()
#endif

class PokerPRT : public MLibThread
{
public:
	PokerPRT();
	virtual ~PokerPRT();
	bool run();
};
#endif
