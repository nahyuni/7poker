#include "PokerPST.h"
#include "MLibRequest.h"
#include "MLibLog.h"
PokerPST::PokerPST()
{
}
bool PokerPST::run()
{
	while(1)
	{
		m_queuePacketMutex.lock();
		int queue_length = m_queuePacket.size();
		while(queue_length==0)
		{

			m_queuePacketMutex.wait();
			queue_length = m_queuePacket.size();
		}
		deque<MLibHeaderPacket>::iterator packet_pos=m_queuePacket.begin();

		while(packet_pos != m_queuePacket.end())
		{
			// 핑 패킷은 로그를 찍지 않는다.
			int packet = 0;
			memcpy(&packet, packet_pos->getPacketBuffer(), 2); 

			if(packet != PK_HEARTBIT)
			{
				LOGOUT("PokerPST::run - sock fd : %d size : %d", packet_pos->getSocketFd(), packet_pos->getPacketSize());
				LOGOUTHEXA(packet_pos->getPacketBuffer(), packet_pos->getPacketSize());
			}

			BYTE _keyList[16] =	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			size_t size = packet_pos->getPacketSize();
			unsigned int arrSize = size < 16 ? 16 : size;

			unsigned int reSize = arrSize % 16 ? arrSize + (16 - arrSize % 16) : arrSize;
			unsigned char* rbuf = (unsigned char*)malloc(reSize);
			memset(rbuf, 0, sizeof(char)* reSize);
			memcpy(rbuf, packet_pos->getPacketBuffer(), packet_pos->getPacketSize()); 
			packet_pos->setPacketSize(reSize);

			crypt2::DWORD roundKey[32];
			crypt2::SeedRoundKey(roundKey, _keyList);

			for (size_t i = 0; i < reSize / 16; ++i)
				crypt2::SeedEncrypt(rbuf + (i * 16), roundKey);

			//LOGOUT("PokerPST::run - reSize : [%d]", reSize);
			MLibRequest request(packet_pos->getSocketFd());
			request.writen(rbuf, reSize);

			m_queuePacket.pop_front();
			packet_pos=m_queuePacket.begin();
			free(rbuf);
		}
		m_queuePacketMutex.unlock();
	}
	return 1;
}

