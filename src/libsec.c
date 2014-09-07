/*---------------------------------------------------------------------------
 * libsec.c Ver1.2
 * CGIセキュリティライブラリ
 * for UNIX
 *
 * Last modified: 1999/2/11
 *
 * Copyright(C) 1998-99  Yuto Ikeno  All rights reserved.
 *
 * e-mail: mew@onbiz.net
 * homepage: http://www.onbiz.net/~mew/
 * support:  http://www.onbiz.net/~mew/cgi-bin/mewbbs/support/mewbbs.cgi
 * compile:  Solaris	gcc -c libsec.c -lnsl -lsocket -lresolv
 *           FreeBSD	gcc -c libsec.c
 *           IRIX		gcc -c libsec.c
 *-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "libinet.h"

#define TRUE 1
#define FALSE 0

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int ReverseDNS (char *)
引数　　　　：char *addr		ホスト名
戻り値　　　：FALSE		FQDN不正
　　　　　　　TRUE		FQDN正しい
説明　　　　：ホスト名とIPが一致するかどうかを判断する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int ReverseDNS ( void )
{

    char				*remotehost;
    char				*remoteaddr;
    char				*ipaddr;
    char				*hostname;

    remotehost = getenv( "REMOTE_HOST" );

    if(	remotehost != NULL ) {

        ipaddr = nresolve( remotehost );
        if( ipaddr == NULL )
            return TRUE;		/*逆引きできないホスト用にとりあえずTRUE*/

        remoteaddr = getenv( "REMOTE_ADDR" );
        if( remoteaddr == NULL )
            return FALSE;		/*REMOTE_ADDRが取れなければFALSE*/

        if(!strcasecmp( remoteaddr, ipaddr )) {

            hostname = iresolve( ipaddr );
            if( hostname == NULL )
                return TRUE;		/*正引きできないホスト用にとりあえずTRUE*/

            if(strcasecmp( remotehost, hostname ) != 0)
                return FALSE;		/*DNSに登録されているホスト名とREMOTE_HOSTが異なっていればFALSE*/

        } else {	/*REMOTE_ADDRとREMOTE_HOSTから引けるIPアドレスが異なっていればFALSE*/

            return FALSE;

        }
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int isIPAddress (char *)
引数　　　　：char *remote_addr		ホスト名（REMOTE_HOSTを渡す）
戻り値　　　：FALSE		ホストである
　　　　　　　TRUE		IPである
説明　　　　：REMOTE_HOSTに格納された値がホスト名かIPアドレスかを調べる
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int isIPAddress (char *remote_host)
{

    if( inet_addr( remote_host ) == -1 )
        return FALSE;

    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int IsProxy ()
戻り値　　　：FALSE		何か動いてる/ 調査不能
　　　　　　　TRUE		つながらない
説明　　　　：指定したIPの特定ポートへ接続を行う
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int IsProxy ( void )
{

    int					i;
    int					sd;
    char *				srv;

    unsigned int	Port[] = {80, 3128, 8000, 8080, 0};

    if ((srv = getenv ("REMOTE_ADDR")) == NULL)
        return FALSE;

    for( i = 0; Port[i] != 0; i++ ) {
        if((sd = lConnect( srv, Port[i] )) != -1) {
            lClose( sd );
            return FALSE;
        }
        lClose( sd );
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int ViaProxy ()
戻り値　　　：FALSE		PROXY経由
　　　　　　　TRUE		PROXYを経由してない
説明　　　　：環境変数からPROXYサーバ経由を検出
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int ViaProxy ( void )
{

    char *				lpHost;

    if (getenv ("HTTP_VIA") != NULL)
        return (FALSE);

    if (getenv ("HTTP_FORWARDED") != NULL)
        return (FALSE);

    if (getenv ("HTTP_X_FORWARDED_FOR") != NULL)
        return (FALSE);

    if ((lpHost = getenv ("REMOTE_HOST")) != NULL) {
        if (strstr (lpHost, "gate") != NULL)
            return (FALSE);

        if (strstr (lpHost, "cache") != NULL)
            return (FALSE);

        if (strstr (lpHost, "proxy") != NULL)
            return (FALSE);
    }
    return (TRUE);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int isAnonymousProxy ();
戻り値　　　：FALSE		匿名PROXY経由
　　　　　　　TRUE		匿名PROXYを経由してない
説明　　　　：環境変数から匿名PROXYサーバ経由を検出
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int isAnonymousProxy ( void )
{

    if (getenv ("HTTP_X_FORWARDED_FOR") != NULL)
        return (FALSE);

    return TRUE;

}