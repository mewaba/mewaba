/*-----------------------------------------------------------------------------
	debugl.c
		�ǥХå������ϥ饤�֥��

	������			����
	��������		1999/3/23
	�ǽ���������	1999/3/23
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "debugl.h"
#include "libpr.h"

#define FNAME_LEN (256)
static char debuglog[FNAME_LEN] = "./debuglog";

/*-----------------------------------------------------------------------------
ProtoType  : void dInitl( char *fname );
Arguments  : char *fname ... ������Ϥ���ե�����̾
Return     : void
Description: �ǥХå����ν�����ե�����̾�����ꤹ��
             ���δؿ���¹Ԥ��ʤ����ϥ����ȥǥ��쥯�ȥ��debuglog�ˤʤ�
-----------------------------------------------------------------------------*/
void dInitl( char *fname )
{

    if( strlen( fname ) > FNAME_LEN ) {
        perror("Too long filename.(dInit())");
        return;
    }
    strcpy( debuglog, fname );

    return;

}

/*-----------------------------------------------------------------------------
ProtoType  : void dPrintl( char *msg, const char *srcname, const unsigned int srcline );
Arguments  : char *msg ... ���Ϥ����å�����
             const char *srcname ... ��å���������Ϥ��륽�����ե�����̾
                                     �ʸƤӽФ�¦��__FILE__����ꤹ���
             const unsigned int srcline ... ��å���������Ϥ��륽�����ե�����ιԿ�
                                           �ʸƤӽФ�¦��__FILE__����ꤹ���
Return     : void
Description: �ǥХå�������Ϥ���
-----------------------------------------------------------------------------*/
void dPrintl( char *msg, const char *srcname, unsigned int srcline)
{

    FILE *dfp;
    char *buf;
    struct tm *jst;
    time_t ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT��JST*/
    jst = gmtime(&ti);

    buf = (char *)malloc( strlen(msg) + 1 );
    if( buf == NULL ) {
        perror("Memory allocation error.(dPrint())");
        return;
    }
    strcpy( buf, msg );
    removeNewline(buf);

    if( (dfp = fopen( debuglog, "a" )) == NULL ) {
        perror("File open error.(dPrint())");
        free( buf );
        return;
    }

    fprintf( dfp, "[%02d/%02d/%02d:%02d %s:%d] %s\n", jst->tm_mon+1, jst->tm_mday, jst->tm_hour, jst->tm_min, srcname, srcline, buf );
    fclose( dfp );
    free( buf );

    return;

}