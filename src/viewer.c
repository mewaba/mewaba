/*---------------------------------------------------------------------------
 * viewer.c
 * mewBBS 発言表示用
 * for UNIX
 *
 * Last modified 1999/4/17
 *
 * Copyright(C) 1999  Yuto Ikeno  All rights reserved.
 * mailto: mew@onbiz.net
 * http://www.onbiz.net/~mew/
 *-------------------------------------------------------------------------*/

/*--- Define ---*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mewbbs.h"
#include "libcgi.h"
#include "libpr.h"


#define BUFSIZE (1024)

char	body[BUFSIZE];
char	font[BUFSIZE];

/*--- Code ---*/
int getConfig( CF *config )
{

    char	**file2Dim;

    file2Dim = readFile( CONFIG_FILE );
    if( file2Dim == NULL ) {
        fatal_error("設定ファイルが存在しないか、otherに書き込み権限が与えられていません。","<BODY>");
        exit(1);
    }

    if(getCfValueStr(file2Dim, "APTITL", config -> aptitle, MAX_CONFIG_LEN) == NULL)
        strcpy( config -> aptitle, "タイトル未設定" );
    if(getCfValueStr(file2Dim, "BGCOLOR", config -> bgcolor, MAX_CONFIG_LEN) == NULL)
        strcpy( config -> bgcolor, "#FFFFFF" );
    if(getCfValueStr(file2Dim, "BACKGROUND", config -> background, MAX_CONFIG_LEN) == NULL)
        strcpy( config -> background, "" );
    if(getCfValueStr(file2Dim, "TEXT", config -> text, MAX_CONFIG_LEN) == NULL)
        strcpy( config -> text, "#000000" );
    if(getCfValueStr(file2Dim, "LINK", config -> link_color, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> link_color, "0000FF");
    if(getCfValueStr(file2Dim, "VLINK", config -> vlink, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> vlink, "5555FF");
    if(getCfValueStr(file2Dim, "ALINK", config -> alink, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> alink, "FF0000");
    if(getCfValueStr(file2Dim, "TCOLOR", config -> title_color, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> title_color, "CCCCCC");
    if((config -> fweight = getCfValueInt(file2Dim, "FWEIGHT")) == -1)
        config -> fweight = 0;
    if((config -> fsize = getCfValueInt(file2Dim, "FSIZE")) == -1)
        config -> fsize = 3;

    freeTwoDimArray( file2Dim );

    sprintf(body,"<BODY BACKGROUND=\"%s\" BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\">" , config -> background, config -> bgcolor, config -> text, config -> link_color, config -> vlink, config -> alink);
    if(config -> fweight == 0)
        sprintf(font, "<FONT SIZE=%d>", config -> fsize);
    else
        sprintf(font, "<FONT SIZE=%d><B>", config -> fsize);

    return 1;
}

int main(void)
{

    int number = 0;				/*表示する発言番号*/
    int i;						/*カウンタ*/
    char fname[FNAME_LEN];		/*ファイル名作成*/
    char *buffer;				/*バッファ*/
    char **file2Dim;			/*発言ファイルを格納*/
    char **name, **value;		/*QUERY_STRINGのname=value*/
    int count;					/*name=valueの組数*/
    char	Title[BUFSIZE];
    CF		config;

    if(!getConfig( &config )) {
        fatal_error("■ 設定ファイルの読み込みに失敗しました。", body);
        return 1;
    }

    count = getForm(&name, &value);
    buffer = getValue( "number", name, value );
    if( buffer == NULL ) {
        fatal_error("■ システムエラーです。このCGIの実行方法をお確かめください。", body);
        exit(1);
    } else {
        number = atoi( buffer );
        if( number == 0 ) {
            fatal_error("■ システムエラーです。再度実行してください。", body);
            exit(1);
        }
    }

    sprintf( fname,"./file/%d", number );
    file2Dim = readFile( fname );
    if( file2Dim == NULL ) {
        fatal_error("■ システムエラーです。再度実行してください。", body);
        exit(1);
    }

    /*--- フォーム出力 ---*/
    sprintf( Title, "#%d (%s)", number, config.aptitle );
    printPageHeader(Title);
    puts(body);
    printf("　%05d　%s　<A HREF=\"mailto:%s\">%s</A>　　<B>%s</B>\n", number, *(file2Dim + 1), *(file2Dim + 3), *(file2Dim + 2), *(file2Dim + 5));

    printf(
        "<HR>\n"
        "<BLOCKQUOTE>\n"
        "<PRE>%s"
        , font
    );

    for( i = 15; *(file2Dim + i); i++ ) {
        if( **(file2Dim) == '>' )	/*引用行は斜体にする*/
            printf( "<EM>%s</EM>", *(file2Dim + i) );
        else
            fputs( *(file2Dim + i), stdout );
    }

    printUrl( *(file2Dim + 11) );
    printf(
        "</FONT>"
        "</PRE>\n"
        "</BLOCKQUOTE>\n"
        "<HR>\n"
        "</BODY>\n"
        "</HTML>\n"
    );

    freeTwoDimArray( file2Dim );

    if(count > 0)
        freedata(name, value);

    exit(0);
}
