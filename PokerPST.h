#ifndef _Poker_PST_H_
#define _Poker_PST_H_
#include <list>
#include <deque>
#include "MLib.h"
#include "MLibThread.h"
#include "MLibMutex.h"
#include "MLibSingleton.h"
#include "GameData.h"
#include "MLibHeaderPacket.h"

extern MLibMutex m_userListMutex;
extern MLibMutex m_queuePacketMutex;//
extern list<User> m_userList;
extern deque<MLibHeaderPacket> m_queuePacket;

class PokerPST : public MLibThread
{
public:
	PokerPST();
	virtual ~PokerPST() { } ;
	bool run();
};
#endif
