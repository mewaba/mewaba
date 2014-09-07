/*-----------------------------------------------------------------------------
	libinet.c
		インターネット汎用ライブラリ

	作成者			池野
	作成日時		1999/1/25
	最終更新日時	1999/2/20	池野
-----------------------------------------------------------------------------*/
#include <string.h>		/*memset()*/
#include <unistd.h>		/*close()*/
#include <netdb.h>
#ifdef SOLARIS
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libinet.h"

/*-----------------------------------------------------------------------------
プロトタイプ：int lConnect( const char *srv, unsigned long int port );
引数		：const char *srv ... 接続先のサーバ名。またはIPアドレス
			　unsigned long int port ... 接続先のポート番号
返値		：int ... 正常：ソケットディスクリプタ, 異常：-1
説明		：srvとportで指定されたサーバへ接続し、socket descripterを返す。
作成者		：池野
作成日時	：1999/2/7
最終更新日時：1999/2/7
-----------------------------------------------------------------------------*/
int lConnect( const char *srv, unsigned long int port ){

	int		sd;
	unsigned long int	ipaddr;
	struct	sockaddr_in	sv_addr;
	struct	hostent		*shost;

	if( (sd = socket( AF_INET, SOCK_STREAM, 0 )) < 0)	/*ソケット作成*/
		return -1;

	if( inet_addr(srv) == -1 ){		/*名前で指定した場合*/
		if((shost = gethostbyname(srv)) == NULL){
			close( sd );
			return -1;
		}
	}else{							/*IPアドレスの場合*/
		ipaddr = inet_addr(srv);
		if ((shost = gethostbyaddr ((char *)&ipaddr, 4, AF_INET)) == NULL){
			close( sd );
			return -1;
		}
	}

	memset ( (char *)&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	memcpy(&sv_addr.sin_addr, (char *)shost->h_addr, shost->h_length);

	if( connect( sd, (struct sockaddr*)&sv_addr, sizeof(sv_addr) ) < 0){		/*ソケットの接続要求*/
		close( sd );
		return -1;
	}

	return sd;

}

void lClose( int sd ){

	shutdown( sd, 2 );
	close( sd );

	return;

}

/*-----------------------------------------------------------------------------
プロトタイプ：char *nresolve( const char *hostname );
引数		：const char *hostname ... 正引きするホスト名
返値		：char * （成功）... hostnameのIPアドレス
			　       （失敗）... NULL
説明		：hostnameを正引きし、IPアドレスを返す。
			　IPアドレスが引数に渡された場合もそのまま返す。
作成者		：池野
作成日		：1999/2/5
-----------------------------------------------------------------------------*/

char *nresolve( const char *hostname ){

	struct hostent *host;		/*host infomation*/
	struct in_addr addr;		/*IP Address*/
	char	*ipaddr;

	host = gethostbyname( hostname );
	if(host == NULL)
		return NULL;
	memcpy(&addr.s_addr, (char *)host->h_addr, host->h_length);
	ipaddr = inet_ntoa(addr);

	return ipaddr;

}


/*-----------------------------------------------------------------------------
プロトタイプ：char *iresolve( const char *ipaddr );
引数		：const char *ipaddr ... 逆引きするIPアドレス
返値		：char * （成功）... ipaddrのホスト名
			　       （失敗）... NULL
説明		：IPアドレスを逆引きし、ホスト名を返す。
作成者		：池野
作成日		：1999/2/5
-----------------------------------------------------------------------------*/

char *iresolve( const char *ipaddr ){

	struct				hostent *host;
	unsigned long int	naddr;			/*network byte order*/

	naddr = inet_addr( ipaddr );
	host = gethostbyaddr( (char *)&naddr, 4, AF_INET );
	if(host == NULL)
		return NULL;

	return host->h_name;

}

/*-----------------------------------------------------------------------------
プロトタイプ：char *getpeerip( int sd );
引数		：int sd ... 接続元のソケットディスクリプタ
返値		：char * （成功）... 接続元のIPアドレス
			　       （失敗）... NULL
説明		：接続元のIPアドレスを返す。
作成者		：池野
作成日		：1999/2/5
-----------------------------------------------------------------------------*/

char *getpeerip( int sd ){

	struct sockaddr_in addr;
	int namelen;
	char *s_ipaddr;

	namelen = sizeof( addr );
	memset((char*)&addr, 0, sizeof(addr));
	if(getpeername( sd, (struct sockaddr *)&addr, &namelen ) == -1)
		return NULL;
	s_ipaddr = inet_ntoa(addr.sin_addr);	/*network byte order -> IP Address*/

	return s_ipaddr;

}
