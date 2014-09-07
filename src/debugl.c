/*-----------------------------------------------------------------------------
	debugl.c
		デバッグログ出力ライブラリ

	作成者			池野
	作成日時		1999/3/23
	最終更新日時	1999/3/23
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
Arguments  : char *fname ... ログを出力するファイル名
Return     : void
Description: デバッグログの出力先ファイル名を設定する
             この関数を実行しない場合はカレントディレクトリのdebuglogになる
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
Arguments  : char *msg ... 出力するメッセージ
             const char *srcname ... メッセージを出力するソースファイル名
                                     （呼び出し側で__FILE__を指定する）
             const unsigned int srcline ... メッセージを出力するソースファイルの行数
                                           （呼び出し側で__FILE__を指定する）
Return     : void
Description: デバッグログを出力する
-----------------------------------------------------------------------------*/
void dPrintl( char *msg, const char *srcname, unsigned int srcline)
{

    FILE *dfp;
    char *buf;
    struct tm *jst;
    time_t ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT→JST*/
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