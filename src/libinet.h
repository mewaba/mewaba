/*-----------------------------------------------------------------------------
	libinet.h
		インターネット汎用ライブラリヘッダーファイル

	作成者			池野
	作成日時		1999/1/25
	最終更新日時	1999/2/20
-----------------------------------------------------------------------------*/

int lConnect( const char *srv, unsigned long int port );
void lClose( int sd );
char *nresolve( const char *hostname );
char *iresolve( const char *ipaddr );
char *getpeerip( int sd );
