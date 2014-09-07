/*---------------------------------------------------------------------------
 * libsec.c Ver1.2
 * CGI�Z�L�����e�B���C�u����
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
�v���g�^�C�v�Fint ReverseDNS (char *)
�����@�@�@�@�Fchar *addr		�z�X�g��
�߂�l�@�@�@�FFALSE		FQDN�s��
�@�@�@�@�@�@�@TRUE		FQDN������
�����@�@�@�@�F�z�X�g����IP����v���邩�ǂ����𔻒f����
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
            return TRUE;		/*�t�����ł��Ȃ��z�X�g�p�ɂƂ肠����TRUE*/

        remoteaddr = getenv( "REMOTE_ADDR" );
        if( remoteaddr == NULL )
            return FALSE;		/*REMOTE_ADDR�����Ȃ����FALSE*/

        if(!strcasecmp( remoteaddr, ipaddr )) {

            hostname = iresolve( ipaddr );
            if( hostname == NULL )
                return TRUE;		/*�������ł��Ȃ��z�X�g�p�ɂƂ肠����TRUE*/

            if(strcasecmp( remotehost, hostname ) != 0)
                return FALSE;		/*DNS�ɓo�^����Ă���z�X�g����REMOTE_HOST���قȂ��Ă����FALSE*/

        } else {	/*REMOTE_ADDR��REMOTE_HOST���������IP�A�h���X���قȂ��Ă����FALSE*/

            return FALSE;

        }
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint isIPAddress (char *)
�����@�@�@�@�Fchar *remote_addr		�z�X�g���iREMOTE_HOST��n���j
�߂�l�@�@�@�FFALSE		�z�X�g�ł���
�@�@�@�@�@�@�@TRUE		IP�ł���
�����@�@�@�@�FREMOTE_HOST�Ɋi�[���ꂽ�l���z�X�g����IP�A�h���X���𒲂ׂ�
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int isIPAddress (char *remote_host)
{

    if( inet_addr( remote_host ) == -1 )
        return FALSE;

    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint IsProxy ()
�߂�l�@�@�@�FFALSE		���������Ă�/ �����s�\
�@�@�@�@�@�@�@TRUE		�Ȃ���Ȃ�
�����@�@�@�@�F�w�肵��IP�̓���|�[�g�֐ڑ����s��
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
�v���g�^�C�v�Fint ViaProxy ()
�߂�l�@�@�@�FFALSE		PROXY�o�R
�@�@�@�@�@�@�@TRUE		PROXY���o�R���ĂȂ�
�����@�@�@�@�F���ϐ�����PROXY�T�[�o�o�R�����o
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
�v���g�^�C�v�Fint isAnonymousProxy ();
�߂�l�@�@�@�FFALSE		����PROXY�o�R
�@�@�@�@�@�@�@TRUE		����PROXY���o�R���ĂȂ�
�����@�@�@�@�F���ϐ����瓽��PROXY�T�[�o�o�R�����o
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int isAnonymousProxy ( void )
{

    if (getenv ("HTTP_X_FORWARDED_FOR") != NULL)
        return (FALSE);

    return TRUE;

}