#include <list>
#include <deque>
#include "PokerPRT.h"
#include "MLibLog.h"
#include "MLibPacketParser.h"
#include "MLibRequest.h"
#include "PokerPA.h"
#include "Define.h"
#include "GameData.h"
#include "MySqlDB.h"

MLibMutex m_userListMutex;
MLibMutex m_queuePacketMutex;

int DefaultChannel = 0;
int DefaultRoom = 0;

PokerPRT::PokerPRT()
{
}
PokerPRT::~PokerPRT()
{
}

bool PokerPRT::run()
{
	int tcpsock, asock, sizen;
	unsigned char msg[1024];
	int i, e_cnt;
	short Templen = 0;

	struct sockaddr_in 	caddr;
	unsigned int			caddr_len;	
	caddr_len =	sizeof(caddr);

	UINT16				PacketId;
	UINT16				length;	

	tcpsock = (size_t)getParameter();

	while(1)
	{
		if(e_cnt = MLibSingleton<MLibEpoll>::getInstance()->epoll_cli_wait())
	    if(e_cnt == 0) return -1; /* no event , no work */
	    if(e_cnt < 0) 
	    {
		   printf("[ETEST] epoll wait error\n");
		   return -1; /* return but this is epoll wait error */
	    } 

		for( i = 0; i < e_cnt; i++) {
			if(MLibSingleton<MLibEpoll>::getInstance()->getEpollInfo()[i].data.fd == tcpsock)
			{
				if((asock = accept(tcpsock, (struct sockaddr *)&caddr, &caddr_len))<0) {
					if (errno == EINTR)
					{
						printf("EINTR...");
						continue;             
					}
					else
					{
						printf("accept error\n");
						continue;
					}
				}
				MLibSingleton<MLibEpoll>::getInstance()->epoll_cli_add(asock);			
				if (fcntl(asock,F_SETFL,O_NONBLOCK)<0)
				{
					shutdown(asock,2);
					close(asock);
					continue;
				}
			    continue; /* next fd */

			} else {
				memset(msg,0x00,sizeof(msg));
				INT32 socketFD = MLibSingleton<MLibEpoll>::getInstance()->getEpollInfo()[i].data.fd;
				INT32 rcvSize = 0;
				if((rcvSize = recv(socketFD, msg, sizeof(msg), 1024)) <= 0)
				{
					PacketId = PK_ROOMLEAVE;
					length = 4;
					memcpy(&msg[0], &PacketId, 2);
					memcpy(&msg[2], &length, 2);
					MLibPacketParser<PokerPA> Poker(socketFD, msg);
					MLibSingleton<MLibEpoll>::getInstance()->epoll_cli_del(socketFD);
					close(socketFD);
				} else {
					UINT16 protocal;
					memset(&protocal, 0x00, sizeof(protocal));
					memcpy(&protocal, &msg[0], 2);

					if(protocal == PK_CS2CONFIG || protocal == PK_CS2USERDISCONNECT) 
					{
						MLibPacketParser<PokerPA> Poker(MLibSingleton<MLibEpoll>::getInstance()->getEpollInfo()[i].data.fd, msg);
					} 
					else if(protocal > 0x00)
					{

						BYTE _keyList[16] =	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

						crypt2::DWORD roundKey[32];
						crypt2::SeedRoundKey(roundKey, _keyList);
						for (size_t i = 0; i < rcvSize / 16; ++i)
						{
							crypt2::SeedDecrypt((unsigned char*)msg + (i * 16), roundKey);
						}

						memcpy(&Templen, msg + 2, sizeof(short)); //먼저 헤더에 입력된 데이터 길이를 읽음 
						unsigned int arrSize = Templen < 16 ? 16 : Templen;
						unsigned int reSize = arrSize % 16 ? arrSize + (16 - arrSize % 16) : arrSize;

						if (reSize == rcvSize)
						{
							LOGOUT("PokerPA::run - header packet : %d,%d,%d,%d,%d,%d", (int)(msg[0]), (int)(msg[1]), (int)(msg[2]), (int)(msg[3]), (int)(msg[4]), (int)(msg[5]));
							MLibPacketParser<PokerPA> Poker(socketFD, msg);	
						
						}
						else if (reSize < rcvSize) //패킷이 붙어서 들어왔을 경우 
						{
							for (int n = 0; n < 10; n++)
							{
								memcpy(&Templen, msg + 2, sizeof(short)); //먼저 헤더에 입력된 데이터 길이를 읽음 
								arrSize = Templen < 16 ? 16 : Templen;
								reSize = arrSize % 16 ? arrSize + (16 - arrSize % 16) : arrSize;

								if (reSize > rcvSize)
									break;

								LOGOUT("PokerPA BigSize::run - reSize = %d, rcvSize = %d", reSize, rcvSize);
								MLibPacketParser<PokerPA> Poker(socketFD, msg);	

								memmove(msg, msg + reSize, rcvSize - reSize); //읽은 만큼 메모리를 앞으로 민다. 
								rcvSize -= reSize;    //남은 길이 구함
							
								if (rcvSize <= 0)
										break;
							}
						}
					}
				}
			}
		}
	}
}
