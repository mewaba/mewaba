/*---------------------------------------------------------------------------
 * libsec.h
 * CGIセキュリティライブラリ用ヘッダファイル
 * for UNIX
 *-------------------------------------------------------------------------*/

int ReverseDNS ();
int isIPAddress (char *);
int IsProxy ( void );
int ViaProxy ( void );
int isAnonymousProxy ( void );
