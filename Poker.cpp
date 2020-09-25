#include "MLib.h"
#include "MLibLog.h"
#include "MLibSingleton.h"
#include "MLibPacket.h"

#include "Define.h"
#include "PokerPRT.h"
#include "PokerPST.h"
#include "PokerSyn.h"
#include "MySqlDB.h"

int main(int argc, char ** argv)
{

	int tcpsock;
	int retry=0;
	int port = 0;
	struct sockaddr_in laddr;
	int i, efd;

	//���� �α׸� �����Ѵ�.
	LOGOUT("====================================");
	LOGOUT("START!!!!!!");
	LOGOUT("====================================");

	//rand �ʱ�ȭ
	srand(time(NULL));
	pthread_attr_t attr;

    for(int j = 0; j < S_MySql->GetDBConnct(); j++)
	{
		S_MySql->db_init(&S_MySql->dbConn[j], dbServerUser, dbServerPass, dbServerDB, dbServerIP, dbServerPort);
		sprintf(S_MySql->dbConn[j].charset, "%s", "utf8");
		S_MySql->db_connect(&S_MySql->dbConn[j]);
    }
	GameManager::getInstance()->load_config();
	GameManager::getInstance()->newAutoUserAI();
	GameManager::getInstance()->newGameAutoRoom();

	if((tcpsock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		LOGOUT("Poker Socket ERROR");
		return -1;
	}
	memset(&laddr,0x00,sizeof(laddr));

	laddr.sin_family=AF_INET;
	laddr.sin_port = htons(PORT);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	port = PORT;

	for( i=0;i<3;i++)
	{
		if(bind(tcpsock,(struct sockaddr *)&laddr,sizeof(laddr)) <0)
		{
			port++;
			laddr.sin_port = htons(port);
			continue;
		}
		break;
	}
	if(i==3)
	{
		LOGOUT("Binding Error");
		close(tcpsock);
		return -1;	
	}

	int sndBufferSize = 65535;
	if(setsockopt(tcpsock, SOL_SOCKET, SO_RCVBUF, &sndBufferSize, sizeof(sndBufferSize)) < 0)
	{
        LOGOUT("setsockopt() failed");
        exit(1);
	}

	// ���� �ɼ� (��� ����)
	// l_onoff�� 0�̸� �⺻���� TCP�� �����Ѵ�.
	// l_onoff�� 0�� �ƴϰ� l_linger�� 0�̸� close �� �� ���ۿ� �����ִ� ������ ������ ������ ���������.
	// l_onoff�� 0�� �ƴϰ� l_linger�� 0�� �ƴ϶��, close �� �� l_linger �ð���ŭ(�ʴ���)�� ��ٸ� ��
	// �׶����� ���۰� ó������ ������ ������ ���´�.
/*
	struct linger   optval;
	optval.l_onoff = 1;
	optval.l_linger = 0;
	if (setsockopt(tcpsock, SOL_SOCKET, SO_LINGER, (void *)&optval, sizeof(optval)) == -1) {
		LOGOUT("setsockopt(SO_LINGER) fail");
		exit(1);
	}
*/
	int result = 1;
	if (setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &result, sizeof(result)) == -1) {
		LOGOUT("setsockopt(SO_REUSEADDR) fail");
		exit(1);
	}
	
	if(fcntl(tcpsock,F_SETFL, O_NONBLOCK) == -1)
	{
		LOGOUT("FCNTL ERROR");
		close(tcpsock);
		return -1;
	}

	if(listen(tcpsock,5) < 0)
	{
		LOGOUT("LISTEN ERROR");
		close(tcpsock);
		return -1;
	}

		MLibSingleton<MLibEpoll>::getInstance()->Setfd(tcpsock);
		MLibSingleton<MLibEpoll>::getInstance()->epoll_init();

		PokerPRT recv;
		recv.create((int *)tcpsock);
		PokerPST send;
		send.create((int *)tcpsock);
		PokerSyn  sync;
		sync.create((int *)tcpsock);
		pause();
}

