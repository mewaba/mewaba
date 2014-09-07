/*---------------------------------------------------------------------------
 * libcgi.c Ver4.2
 * CGI汎用ユーティリティ
 *
 * Last update : 1999/4/11
 *
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "libcgi.h"
#include "libstr.h"
#include "libpr.h"

/*--- Code ---*/

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int selectBrouser( void );
返値		：int ... Internet Explorer - 1
			　        Netscape Navegator - 2
			　        エラー or その他 - 0
説明　　　　：使用しているブラウザを判別する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

int selectBrouser( void )
{

    char	*useragent;

    useragent = getenv( "HTTP_USER_AGENT" );
    if( useragent == NULL )
        return 0;


    if(strstr( useragent, "Mozilla" ) != NULL) {
        if(strstr( useragent, "MSIE" ) != NULL)		/*IE*/
            return 1;
        else										/*NN*/
            return 2;
    }
    return 0;

}


char tochar(char *x)
{

    register char c;

    c  = (x[0] >= 'A' ? ((x[0] & 0xdf) - 'A') + 10 : (x[0] - '0'));
    c *= 16;
    c += (x[1] >= 'A' ? ((x[1] & 0xdf) - 'A') + 10 : (x[1] - '0'));
    return(c);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void decode(char *)
引数　　　　：char *url		エンコードされた文字列
説明　　　　：urlをデコードする。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void decode(char *url)
{

    register int i, j;

    for(i = 0, j = 0; url[j]; ++i, ++j) {
        if((url[i] = url[j]) == '%') {
            url[i] = tochar(&url[j + 1]);
            j += 2;
        } else if (url[i] == '+')
            url[i] = ' ';
    }
    url[i] = '\0';
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int getdata (char ***, char ***)
引数　　　　：char ***dname		フォームデータのnameを確保するための領域へのポインタ
　　　　　　　char ***dvalue	フォームデータのvalueを確保するための領域へのポインタ
戻り値　　　：int				name=vallueの組数
説明　　　　：フォームのデータを受け取り、nameとvalueに分割する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getFormData( void )
{

    char *qstring = NULL;			/*stdin data*/
    char *qs;
    char *method;			/*REQUEST_METHOD*/
    int ictl;				/*CONTENT_LENGTH*/

    method = getenv("REQUEST_METHOD");	/*方式を調べる*/
    if(method == NULL) {
        return NULL;
    } else if(!strcmp(method, "POST")) {	/*POSTの時*/

        ictl = atoi(getenv("CONTENT_LENGTH"));
        if(ictl == 0)	/*受け取るデータが存在しない*/
            return NULL;
        if((qstring = (char *)malloc( sizeof(char) * (ictl + 1))) == NULL)
            return NULL;
        memset( (char *)qstring, '\0', sizeof(qstring) );
        if((fread(qstring, ictl, 1, stdin)) != 1)
            return NULL;
        qstring[ictl] = '\0';

    } else if(!strcmp(method, "GET")) {			/*GETの時*/
        qs = getenv("QUERY_STRING");
        if(qs == NULL) {
            return NULL;
        } else {
            ictl = strlen(qs);
            if((qstring = (char *)malloc(sizeof(char) * (ictl + 1))) == NULL)
                return NULL;
            strcpy(qstring,qs);
        }
    }

    return qstring;

}

int getForm( char ***dname, char ***dvalue )
{

    char			*formdata;
    unsigned int	count;

    formdata = getFormData();
    if( formdata == NULL )
        return -1;

    count = dataSeparater( dname, dvalue, formdata );
    if( count < 0 )
        return -1;

    return count;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int freedata (char **, char **, int)
引数　　　　：char **name	メモリ解放対象の二次配列
　　　　　　　char **value	メモリ解放対象の二次配列
　　　　　　　int count		name=valueの組数
説明　　　　：nameとvalueのメモリを解放する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void freedata( char **name, char **value )
{

    int i;

    if( name != NULL ) {
        for(i = 0; *(name + i); i++) {
            if(*(name + i))
                free(*(name + i));
        }
        if(name)
            free(name);
    }

    if( value != NULL ) {
        for(i = 0; *(value + i); i++) {
            if(*(value + i))
                free(*(value + i));
        }
        if(value)
            free(value);
    }

    return;

}

char **getCookies( void )
{

    char			*http_cookie;	/*HTTP_COOKIE*/
    char			*buf_http_cookie;
    char			**tmp = NULL;
    char			buf[BUFSIZE];
    int				flag = 1;		/*strspl()で用いるフラグ*/
    unsigned int	record_num;

    http_cookie = getenv( "HTTP_COOKIE" );
    if( http_cookie == NULL )	/*HTTP_COOKIEが存在しない場合*/
        return NULL;

    buf_http_cookie = (char *)malloc( strlen(http_cookie) + 2 );
    if( buf_http_cookie == NULL )
        return NULL;
    sprintf( buf_http_cookie, " %s", http_cookie );

    record_num = 0;
    while( flag ) {
        tmp = (char **)realloc( tmp, sizeof(char **) * (record_num + 1) );
        if( tmp == NULL )
            return NULL;

        *(tmp + record_num) = (char *)malloc( strlen(buf_http_cookie) + 1 );
        if( *(tmp + record_num) == NULL )
            return NULL;

        flag = strspl( *(tmp + record_num), buf_http_cookie, ';' );
        strspl(buf, *(tmp + record_num), ' ');
        record_num++;
    }

    tmp = (char **)realloc( tmp, sizeof(char **) * (record_num + 1) );
    if( tmp == NULL )
        return NULL;
    *(tmp + record_num) = NULL;		/*終端にNULL*/
    free( buf_http_cookie );

    return tmp;

}

char *getCookieRecord( char *ckname , char **cookies )
{

    unsigned int	i;
    char			*buf = NULL;

    i = 0;
    while( *(cookies + i) ) {
        buf = (char *)malloc( strlen(*(cookies + i) + 1) );
        if( buf == NULL )
            return NULL;
        strspl( buf, *(cookies + i), '=' );
        if( !strcmp( buf, ckname ) ) {
            free( buf );
            return *(cookies + i);
        }
        i++;
    }
    free( buf );
    return NULL;

}

int dataSeparater( char ***name, char ***value, char *data)
{

    int count;				/*ddname=dvalueの組数*/
    char **tmpname = NULL,**tmpval = NULL;	/*dnameとdvalueを格納する２次配列*/
    int flag = 1;

    /*----- 分割＆デコード -----*/
    count = 0;
    while( flag ) {
        tmpval = (char **)realloc( tmpval, sizeof(char **) * (count + 1));
        if( tmpval == NULL )
            return -1;
        *(tmpval + count) = (char*)malloc( strlen(data) + 1 );
        if( *(tmpval + count) == NULL )
            return -1;
        memset( (char *)*(tmpval + count), '\0', sizeof(*(tmpval + count)) );

        tmpname = (char **)realloc( tmpname, sizeof(char **) * (count + 1));
        if( tmpname == NULL )
            return -1;
        *(tmpname + count) = (char*)malloc( strlen(data) + 1 );
        if( *(tmpname + count) == NULL )
            return -1;
        memset( (char *)*(tmpname + count), '\0', sizeof(*(tmpname + count)) );

        flag = strspl(*(tmpval + count), data, '&');
        decode(*(tmpval + count));
        strspl(*(tmpname + count), *(tmpval + count), '=');

        count++;
    }
    tmpval = (char **)realloc( tmpval, sizeof(char **) * (count + 1));
    if( tmpval == NULL )
        return -1;
    *(tmpval + count) = NULL;

    tmpname = (char **)realloc( tmpname, sizeof(char **) * (count + 1));
    if( tmpname == NULL )
        return -1;
    *(tmpname + count) = NULL;

    *name = tmpname;
    *value = tmpval;

    return count;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int getcookiesdata (char ***, char ***, char *)
引数　　　　：char ***cname		cookieのnameを確保するための領域へのポインタ
　　　　　　　char ***cvalue	cookieのnameを確保するための領域へのポインタ
　　　　　　　char *name		取得するcookieの名前
戻り値　　　：int				name=valueの組数
説明　　　　：nameで指定されたcookieのnameとvalueを分割し格納する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCookieData(char ***cname, char ***cvalue, char *name)
{

    char			**cookies;
    char			*record;
    unsigned int	count;

    cookies = getCookies();
    if( cookies == NULL )
        return -1;

    record = getCookieRecord( name, cookies );
    if( record == NULL )
        return -1;

    count = dataSeparater( cname, cvalue, record );
    freeTwoDimArray( cookies );
    if( count < 0 )
        return -1;

    return count;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void fatal_error (const char *, const char *)
引数　　　　：const char *message		エラーメッセージとして出力する文字列
　　　　　　　const char *body			HTMLのBODY
説明　　　　：messageで指定したエラーメッセージをBODYの色ページへ表示する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void fatal_error(const char* message, const char* body)
{

    printPageHeader( "致命的エラー" );
    puts(body);
    puts("<BLOCKQUOTE><BR><BR>");
    puts(message);
    puts("</BLOCKQUOTE></BODY></HTML>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int getCountInt (const char *)
引数　　　　：const char *filename	対象のファイル
戻り値　　　：int					取得したカウンタ
説明　　　　：filenameからint型の数を読み込み返す。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCountInt(const char* file_path)
{

    int count;
    char buf[6];
    FILE *fp;

    if((fp = fopen(file_path,"r")) == NULL)
        fatal_error("カウンターファイルが開けません","<BODY>");

    fgets(buf,6,fp);
    count = atoi(buf);
    fclose(fp);

    return count;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int putCountInt( const char *, int )
引数　　　　：const char *filename	対象のファイル
　　　　　　　int count				int型の数
説明　　　　：filenameで指定されたファイルへcountを書き込む。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int putCountInt(const char* file_path, int count)
{

    FILE *fp;

    if((fp = fopen(file_path,"w")) == NULL)
        return 1;

    if(f_lock( fileno(fp) ) == -1 )
        return 1;

    fprintf(fp,"%05d\n",(count+1));

    if(f_unlock( fileno(fp) ) == -1 )
        return 1;

    fclose(fp);

    return 0;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *check_url (char *)
引数　　　　：char *url		チェック対象のURL
戻り値　　　：char *		チェック後の文字列
説明　　　　：urlが正しい形式であるか調べる
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int check_url(char *url)
{

    int i;
    int count;

    i = 0;
    count = 0;
    while(url[i]) {
        if(url[i++] == '.')
            count++;
    }
    if(i > 8) {
        if(count < 1)
            return 0;
        if(strstr(url, "://") == NULL)
            return 0;
        if(strstr(url, ",") != NULL)
            return 0;
    } else {
        return 2;
    }
    return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void setCfValue(const char *, const char *, const char *)
引数　　　　：const char *cfFname		コンフィギュレーションファイル名
　　　　　　　const char *keyName		値をセットするキーの名前
　　　　　　　const char *keyValue		セットする値
説明　　　　：cfFnameで指定されたコンフィギュレーションファイルを読み込み、
　　　　　　　該当するkeyNameの行を探し、値をKeyValueに変更する。
　　　　　　　ただし、コンフィギュレーションファイルの書式は以下の通り
　　　　　　　また、一つのファイル中に該当するキーは一つしか存在しないものと
　　　　　　　する。
=============================================================================
KeyName1=KeyValue1
KeyName2=KeyValue2
KeyName3=KeyValue3
=============================================================================
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void setCfValue(const char *cfFname, const char *KeyName, const char *KeyValue)
{

    char			tmp[BUFSIZE];
    char			tmp2[BUFSIZE];
    char			**bufRead;
    FILE			*wfp;
    unsigned int	flag = 0;
    unsigned int	i;

    if((bufRead = readFile(cfFname)) != NULL) {
        if((wfp = fopen(cfFname, "w")) == NULL) {
            fatal_error("ファイルのオープンに失敗しました。(setCfValue()-2)", "<BODY>");
            freeTwoDimArray(bufRead);	/*領域解放*/
            exit(-1);
        }

        for (i = 0; *(bufRead + i); i++) {
            strcpy(tmp2, *(bufRead + i));
            strspl(tmp, tmp2, '=');
            if(!strcmp(tmp, KeyName)) {
                fprintf(wfp, "%s=%s\n", KeyName, KeyValue);
                flag = 1;
            } else {
                fputs(*(bufRead + i), wfp);
            }
        }
        fclose(wfp);
        freeTwoDimArray(bufRead);	/*領域解放*/
        if(flag == 0)		/*該当するキーが存在しなかった場合はそのキーを作成する*/
            createCfKey(cfFname, KeyName, KeyValue);

    } else {
        createCfKey(cfFname, KeyName, KeyValue);
    }
    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void createCfKey(const char *, const char *, const char *)
引数　　　　：const char *cfFname		コンフィギュレーションファイル名
　　　　　　　const char *keyName		生成するキーの名前
　　　　　　　const char *keyValue		生成するキーに対する値
説明　　　　：cfFnameで指定されたコンフィギュレーションファイルの最終行に
　　　　　　　KeyName=KeyValueを追加する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void createCfKey(const char *cfFname, const char *KeyName, const char *KeyValue)
{

    FILE			*afp;

    if((afp = fopen(cfFname, "a")) == NULL) {
        fatal_error("ファイルのオープンに失敗しました。(setCfValue()-2)", "<BODY>");
        exit(-1);
    }

    fprintf(afp, "%s=%s\n", KeyName, KeyValue);
    fclose(afp);

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *getFormdataValue(char *, char **, char **, int)
引数　　　　：char *name		値を取得したいname
　　　　　　　char **dname		フォームデータのnameを格納した二次配列
　　　　　　　char **dvalue		フォームデータのvalueを格納した二次配列
返り値　　　　char *			引数のnameに対応するvalue
　　　　　　　　　　　　　　　　nameが存在しなかった場合はNULL
説明　　　　：引数のnameで指定したvalueを返します。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getValue(char *name, char **dname, char **dvalue)
{

    int	i;

    if( dname != NULL && dvalue != NULL ) {
        for(i = 0; *(dname + i); i++) {
            if(!strcmp(*(dname + i), name))
                return *(dvalue + i);
        }
    }
    return NULL;

}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *getValueAlloc(char *, char **, char **, int)
引数　　　　：char *name		値を取得したいname
　　　　　　　char **dname		フォームデータのnameを格納した二次配列
　　　　　　　char **dvalue		フォームデータのvalueを格納した二次配列
　　　　　　　int  count		フォームデータname=valueの組数
返り値　　　　char *			引数のnameに対応するvalue
　　　　　　　　　　　　　　　　nameが存在しなかった場合は\0を代入して返す。
説明　　　　：引数のnameで指定したvalueを返します。getValueとの違いは領域を確
　　　　　　　保して返すかどうかである。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getValueAlloc(char *name, char **dname, char **dvalue)
{

    int	i;
    char	*buf = NULL;

    buf = (char *)malloc(sizeof(char) * 2);
    memset( (char *)buf, '\0', sizeof(buf));

    if( dname != NULL && dvalue != NULL ) {
        for(i = 0; *(dname + i); i++) {
            if(!strcmp(*(dname + i), name)) {
                buf = (char *)realloc(buf, strlen(*(dvalue + i)) + 1);
                strcpy(buf, *(dvalue + i));
            }
        }
    }
    return buf;

}


/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *getCfValueStr(FILE *, const char *);
引数　　　　：FILE *rfp				該当するファイルへのポインタ
返り値　　　　const char *KeyName	抽出したいキーの名前
説明　　　　：rfpで指定されたファイルからKeyNameに該当するValueを返す。
　　　　　　　ただし、文字列のValueに限ります。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getCfValueStr( char **file2Dim, const char *KeyName, char *KeyValue, size_t size)
{

    unsigned int	i;
    char			*tmpKeyName;
    char			*buf;

    memset( (char *)KeyValue, 0, sizeof(KeyValue) );
    for(i = 0; *(file2Dim + i) ; i++) {
        buf = (char *)malloc(strlen(*(file2Dim + i)) + 1);
        strcpy( buf, *(file2Dim + i) );
        tmpKeyName = (char *)malloc(strlen(*(file2Dim + i)) + 1);
        strspl(tmpKeyName, buf, '=');
        if(!strcmp(KeyName, tmpKeyName)) {
            removeNewline(buf);
            if( strlen(buf) < 1 )
                return NULL;
            strncpy(KeyValue, buf, size);
            free( tmpKeyName );
            free( buf );
            return KeyValue;
        }
        free( tmpKeyName );
        free( buf );
    }
    return NULL;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int getCfValueInt(FILE *, const char *);
引数　　　　：FILE *rfp				該当するファイルへのポインタ
返り値　　　　const char *KeyName	抽出したいキーの名前
説明　　　　：rfpで指定されたファイルからKeyNameに該当するValueを返す。
　　　　　　　ただし、int型のValueに限ります。存在しない場合は-1
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCfValueInt(char **file2Dim, const char *KeyName)
{

    unsigned int	i;
    char			buf[BUFSIZE];

    if(getCfValueStr( file2Dim, KeyName, buf, 6) == NULL) {
        return -1;
    } else {
        for( i = 0; i < (strlen(buf) - 1); i++) {
            if( 0x30 > buf[i] ||  0x39 < buf[i] )
                return -1;
        }
        return atoi( buf );
    }

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *myGetEnv(const char *)
引数　　　　：const char *envName	環境変数名
説明　　　　：envNameで指定した環境変数を取得した後、別の領域に値をコピーして、
　　　　　　　その領域へのポインタを返す。環境変数の取得に失敗した場合は、
　　　　　　　「Unknown」が代入される。また、この関数の使用後は該当する領域に
　　　　　　　対して、必ずfreeEnvironment()をしなければならない。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *myGetEnv(const char *envName)
{

    char *env;		/*環境変数*/
    char *buf;		/*BUFFER*/

    if((buf = getenv(envName)) == NULL) {	/*環境変数の取得に失敗した時*/
        env = (char *)malloc(sizeof(char) * 10);
        strcpy(env, "Unknown");
    } else {
        env = (char *)malloc(strlen(buf) + 1);
        strcpy(env, buf);
    }

    return env;

}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void freeEnvironment(char *)
説明　　　　：環境変数格納用に確保された領域を解放する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void freeEnvironment(char *env)
{

    if(env)
        free(env);

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void printPageHeader(const char *);
引数　　　　：const char *		ページのタイトル
説明　　　　：CGIヘッダとHTMLヘッダを出力する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void printPageHeader(const char *pageTitle)
{

    printf("Content-Type: text/html\n\n");
    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"
           "<!--\n"
           " Programed by Yuto Ikeno.\n"
           " Copyright(C) 1996-99 Myu's Lab. All rights reserved.\n"
           " mailto: mew@onbiz.net\n"
           " http://www.onbiz.net/~mew/\n"
           "-->\n"
           "<HTML>\n\n"
           "<HEAD>\n"
           "<META HTTP-EQUIV=\"Content-Type\"  CONTENT=\"text/html; CHARSET=Shift_JIS\">\n"
           "<LINK REV=MADE HREF=\"mailto:mew@onbiz.net\">\n"
           "<TITLE>%s</TITLE>\n"
           "</HEAD>\n\n" ,pageTitle);

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void printUrl(char *)
引数　　　　：char *url		出力するURL
説明　　　　：参照するURLを表示する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void printUrl(char *url)
{

    int		count;

    for(count = 0 ; ; ) {
        if(*(url + count) == '\0')
            break;
        else
            count++;
    }
    if(count > 10)
        printf("\n\n<B>■ 参照：<A HREF=\"%s\" TARGET=\"_new\">%s</A></B></FONT>",url,url);

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *rmHtmlTag(char *);
引数　　　　：char *str		加工する文字列
説明　　　　：strからHTMLタグを除去する
　　　　　　　例）<B>あいうえお</B> → あいうえお
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *rmHtmlTag(char *str)
{

    unsigned int	flag = 0;
    char			*buf;
    char			*p = str, *p2;

    buf = (char *)malloc(strlen(str) + 1);
    p2 = buf;

    while(*str) {
        if(*str == '<') {
            flag = 1;
        } else if(flag == 1) {
            if(*str == '>')
                flag = 0;
        } else {
            *buf++ = *str;
        }
        str++;
    }
    *buf = '\0';
    buf = p2;
    str = p;

    return buf;
}

/*End of file*/
