/*---------------------------------------------------------------------------
 * mewbbs.c Ver4.94a
 * 多機能掲示板
 * for UNIX
 *
 * Last modified: 1999/4/24
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail: mew@onbiz.net
 * homepage: http://www.onbiz.net/~mew/
 * support:  http://www.onbiz.net/~mew/cgi-bin/mewbbs/support/mewbbs.cgi
 *-------------------------------------------------------------------------*/

/*--- define ---*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>		/*signal(), SIGSEGV*/
#include "mewbbs.h"
#include "libcgi.h"
#include "libstr.h"
#include "libsec.h"
#include "libpr.h"
#include "debugl.h"

#ifdef SOLARIS
#include <crypt.h>
#endif

#ifdef LINUX2
#include <crypt.h>
#endif

#define BUFSIZE (1024)
#define PRF_SIZE (1024)
#define	FALSE			0
#define	TRUE			1

static const char	*weekday[]= {"日","月","火","水","木","金","土"}; /* 表示する曜日の方式 */
static const char	*eweekday[]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"}; /* 表示する曜日の方式 */
static const char	*emonth[]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char	*atype[]= {"write","reload","home","next","res","del","fadmin","fchcfg","chcfg","chsec","fundel","fdel","undel","correction","fcorinput","fcorselect","fchpass","chpass","fsecurity", "delhosts", "addhost",NULL,};
char				master[PRF_SIZE];			/*管理者パスワード*/
char				scurl[PRF_SIZE];			/*スクリプトのURL*/
char				domain[PRF_SIZE];			/*Cookie発行用のDOMAIN*/
char				ckname[PRF_SIZE];			/*Cookie発行用のNAME*/
int					now;						/*現在表示中の最新記事番号*/

char	body[PRF_SIZE];		/*表示色(HTMLのBODYタグ)*/
char	font[PRF_SIZE];


/*-----------------------------------------------------------------------------
プロトタイプ：void fault_abort( int sig );
引数		：int sig		発生したシグナル
説明		：発生したシグナルを受け、exitする。（このスクリプトではSIGSEGV）
-----------------------------------------------------------------------------*/
void fault_abort( int sig )
{

    fatal_error(
        "スクリプトに致命的なエラーが発生しました。再度実行してください。<BR>\n"
        "再実行後も同じメッセージが表\示されるであれば、恐れ入りますが管理者までご連絡ください。<BR>\n"
        "<BR>\n"
        "Myu's Lab.: http://www.onbiz.net/~mew/<BR>\n", body);
    exit(1);

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void addHost (int, char **, char **)
引数　　　　：int count			フォームで送信されたname=valueの数
　　　　　　　char **name 		フォームで送信されたname
　　　　　　　char **value 		フォームで送信されたvalue
説明　　　　：投稿禁止ホストを追加する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int addHost( char **name, char **value )
{

    int		i;

    DPRINTL("Start: addHost()");
    for(i = 0; *(name + i); i++) {
        if(!strcmp(*(name + i),"host")) {
            if(strlen(*(value + i)) < 1) {
                fatal_error("■ 追加するホストが入力されていません。", body);
                return FALSE;
            }
            addLine(DENIED_LIST, *(value + i));
        }
    }
    DPRINTL("End: addHost()");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void deleteHosts(int, char **)
引数　　　　：char **name		フォームで送られてきた削除対象記事番号
説明　　　　：投稿禁止ホストの削除
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int deleteHosts( char **name, char **value )
{

    int		i;				/*カウンタ*/
    int		delnumber;		/*削除対象行*/
    int		delcount = 0;	/*何行消したか*/
    int		flg_cnt = 0;

    for(i = 0; *(name + i); i++) {
        if( **(name + i) == '_' && *(*(name + i) + 1) == '_' )
            flg_cnt++;
    }
    if( flg_cnt == 0 ) {
        fatal_error("■ 削除する投稿禁止ホストを選択してください。", body);
        return FALSE;
    }

    for(i = 0; *(name + i); i++) {
        if((delnumber = atoi(*(value + i))) == 0)
            continue;
        delLine(DENIED_LIST, delnumber - delcount);
        delcount++;
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *getOption(char **, int)
引数　　　　：char **name		フォームの内容から取得された動作オプション
　　　　　　　int count			フォームの内容の個数
説明　　　　：フォームから送信された内容から動作オプションを取得し、返す。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getOption( char **name )
{

    int		i,j;

    for(i = 0; *(name + i); i++) {
        for(j = 0; atype[j]; j++) {
            if(!strcmp(atype[j], *(name + i)))
                return (*(name + i));
        }
    }
    return NULL;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void setConfig (CF)
説明　　　　：ScriptのURL(scurl)、Cookie発行時のNAME(ckname)、DOMAIN(domain)を設定
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void setConfig(CF config)
{

    char			*svName;
    char			*scName;

    svName = getenv("SERVER_NAME");
    if( svName == NULL )
        svName = "Unknown";

    scName = getenv("SCRIPT_NAME");
    if( scName == NULL )
        scName = "Unknown";

    sprintf(scurl, "http://%s%s",svName,scName);
    strcpy(domain, svName);
    trLower(scurl);
    strcpy(ckname, scurl);

    sprintf(body,"<BODY BACKGROUND=\"%s\" BGCOLOR=\"%s\" TEXT=\"%s\" LINK=\"%s\" VLINK=\"%s\" ALINK=\"%s\">" ,config.background ,config.bgcolor ,config.text ,config.link_color ,config.vlink ,config.alink);
    if(config.fweight == 0)
        strcpy(font, "");
    else
        strcpy(font, "<STRONG>");

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void getConfig(CF *)
説明　　　　：mewbbs.cfから設定を読み込む。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getConfig(CF *config)
{

    char	**file2Dim;

    file2Dim = readFile( CONFIG_FILE );
    if( file2Dim == NULL ) {
        fatal_error("設定ファイルが存在しないか、otherに書き込み権限が与えられていません。","<BODY>");
        return FALSE;
    }

    if(getCfValueStr( file2Dim, "APTITL", config -> aptitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> aptitle, "タイトル未設定");
    if(getCfValueStr( file2Dim, "MAINTITL", config -> maintitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> maintitle, "タイトル未設定");
    if(getCfValueStr( file2Dim, "SUBTITL", config -> subtitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> subtitle, "サブタイトル未設定");
    if((config -> max = getCfValueInt( file2Dim, "MAX")) == -1)
        config -> max = 10;
    if((config -> tag = getCfValueInt( file2Dim, "TAG")) == -1)
        config -> tag = 0;
    if(getCfValueStr( file2Dim, "BACKGROUND", config -> background, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> background, "");
    if(getCfValueStr( file2Dim, "BGCOLOR", config -> bgcolor, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> bgcolor, "FFFFFF");
    if(getCfValueStr( file2Dim, "TEXT", config -> text, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> text, "000000");
    if(getCfValueStr( file2Dim, "LINK", config -> link_color, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> link_color, "0000FF");
    if(getCfValueStr( file2Dim, "VLINK", config -> vlink, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> vlink, "5555FF");
    if(getCfValueStr( file2Dim, "ALINK", config -> alink, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> alink, "FF0000");
    if(getCfValueStr( file2Dim, "TCOLOR", config -> title_color, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> title_color, "CCCCCC");
    if((config -> flag_write = getCfValueInt( file2Dim, "WRITE")) == -1)
        config -> flag_write = 1;
    if((config -> fweight = getCfValueInt( file2Dim, "FWEIGHT")) == -1)
        config -> fweight = 0;
    if((config -> fsize = getCfValueInt( file2Dim, "FSIZE")) == -1)
        config -> fsize = 3;
    if((config -> regmax = getCfValueInt( file2Dim, "REGMAX")) == -1)
        config -> regmax = 1000;
    if(getCfValueStr( file2Dim, "HOME_URL", config -> home_url, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> home_url, "http://www.onbiz.net/~mew/");
    if((config -> proxy = getCfValueInt( file2Dim, "PROXY")) == -1)
        config -> proxy = 2;
    if((config -> hostchk = getCfValueInt( file2Dim, "HOSTCHK")) == -1)
        config -> hostchk = 1;

    freeTwoDimArray( file2Dim );
    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void chConfig(char *)
引数　　　　：char **value		フォームから送信された設定内容
説明　　　　：cfの書き換えをする。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int chConfig(char **value)
{

    char *temp;
    unsigned int	i;

    for ( i = 0; *(value + i); i++ ) {
        temp = replacestring(*(value + i), "\"", "'");
        if( temp != NULL) {
            free(*(value + i));
            *(value + i) = temp;
        }
    }

    if(atoi(*(value + 2)) > 50) {
        fatal_error("■ 一度に表\示する記事数は1～50の間に設定してください。" ,body);
        return FALSE;
    }
    setCfValue(CONFIG_FILE, "APTITL", *value);
    setCfValue(CONFIG_FILE, "MAINTITL", *(value + 1));
    setCfValue(CONFIG_FILE, "SUBTITL", *(value + 2));
    setCfValue(CONFIG_FILE, "MAX", *(value + 3));
    setCfValue(CONFIG_FILE, "TAG", *(value + 4));
    setCfValue(CONFIG_FILE, "WRITE", *(value + 5));
    setCfValue(CONFIG_FILE, "BACKGROUND", *(value + 6));
    setCfValue(CONFIG_FILE, "BGCOLOR", *(value + 7));
    setCfValue(CONFIG_FILE, "TEXT", *(value + 8));
    setCfValue(CONFIG_FILE, "LINK", *(value + 9));
    setCfValue(CONFIG_FILE, "VLINK", *(value + 10));
    setCfValue(CONFIG_FILE, "ALINK", *(value + 11));
    setCfValue(CONFIG_FILE, "TCOLOR", *(value + 12));
    setCfValue(CONFIG_FILE, "FWEIGHT", *(value + 13));
    setCfValue(CONFIG_FILE, "FSIZE", *(value + 14));
    setCfValue(CONFIG_FILE, "REGMAX", *(value + 15));
    setCfValue(CONFIG_FILE, "HOME_URL", *(value + 16));

    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void chSecurity(char **)
引数　　　　：char **value		フォームから送信された設定内容
説明　　　　：cfの書き換えをする。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void chSecurity(char **namem, char **valuem, int count)
{

    char		*strProxy;
    char		*strHostchk;

    strProxy = getValue( "PROXY", namem, valuem );
    if( strProxy == NULL ) {
        setCfValue(CONFIG_FILE, "PROXY", "2");
    } else {
        if(atoi( strProxy ) == 1)
            setCfValue(CONFIG_FILE, "PROXY", "1");
        else if(atoi( strProxy ) == 2)
            setCfValue(CONFIG_FILE, "PROXY", "2");
        else
            setCfValue(CONFIG_FILE, "PROXY", "0");
    }

    strHostchk = getValue( "HOSTCHK", namem, valuem );
    if( strHostchk == NULL ) {
        setCfValue(CONFIG_FILE, "HOSTCHK", "0");
    } else {
        if(atoi( strHostchk ) == 1)
            setCfValue(CONFIG_FILE, "HOSTCHK", "1");
        else
            setCfValue(CONFIG_FILE, "HOSTCHK", "0");
    }

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void getCookies(int *, char **, char **, char **)
引数　　　　：int *ctype		確認フォームのタイプへのポインタ
　　　　　　　char **handle		投稿者ハンドルへポインタ
　　　　　　　char **address	投稿者メールアドレスへのポインタ
　　　　　　　char **passwd		投稿者が入力したパスワードへのポインタ
説明　　　　：Cookieから上記の引数をの値を調べ、セットする。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*--handle, address, passwdは呼び出し側関数にて領域解放が必要--*/
void getCookieUser(int *ctype, char **handle, char **address, char **passwd)
{

    int		count;				/*Cookieを分割したname*valueの組数*/
    char	**cname = NULL, **cvalue = NULL;	/*Cookieを分割した値*/
    char	*temp;

    count = getCookieData(&cname, &cvalue, ckname);

    *handle = getValueAlloc("name", cname, cvalue);
    *address = getValueAlloc("e-mail", cname, cvalue);
    *passwd = getValueAlloc("passwd", cname, cvalue);

    *ctype = 0;
    if((temp = getValue("confirm", cname, cvalue)) != NULL) {
        if(atoi(temp) == 1)
            *ctype = 1;
    }
    freedata(cname, cvalue);

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int getCookieNew(int)
返値　　　　：int...既読位置
説明　　　　：既読位置をクッキーから読み込み、返す
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCookieNew(void)
{

    char	*temp;
    int		old_number = 0;
    int		count;				/*Cookieを分割したname*valueの組数*/
    char	**cname = NULL, **cvalue = NULL;	/*Cookieを分割した値*/
    char	c_name[BUFSIZE];	/*未読ポインタ保存用Cookieの名前*/

    old_number = 0;
    sprintf(c_name,"n%s",ckname);
    count = getCookieData(&cname, &cvalue, c_name);
    if( count > 0 ) {
        temp = getValue( "new", cname, cvalue );
        if( temp != NULL )
            old_number = atoi( temp );
        freedata(cname, cvalue);
    }
    return old_number;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void checkReferer(void)
説明　　　　：SCURLと書き込みされてきたURLを比較し、異なるURLからの場合は投稿
　　　　　　　を禁止する。（外部フォームからの登録禁止）
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int checkReferer(void)
{

    char	*referer;		/*HTTP_REFERER*/
    char	*buffer;
    char	*buf_ref;

    referer = getenv("HTTP_REFERER");

    if(referer == NULL || *referer == '\0') {
        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 100)", body);
        return FALSE;
    } else {
        buffer = (char *)malloc(strlen(referer) + 1);
        buf_ref = (char *)malloc(strlen(referer) + 1);
        strcpy(buf_ref, referer);
        strspl(buffer,buf_ref,'?');
        decode(buffer);

        if(strcasecmp(buffer,scurl) != 0) {
            printPageHeader("外部フォームからの書き込み禁止");
            puts(body);
            printf("<BR>あなたが書き込みされたアドレスは、<STRONG>%s</STRONG>です。",buffer);
            printf("<BR>申\し訳ありませんが<A HREF=\"%s\"><STRONG>%s</STRONG></A>から書き込みをお願いします。",scurl,scurl);
            return FALSE;
        }
        free(buffer);
        free(buf_ref);
    }

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char createTitle(char *)
引数　　　　：char *com		元記事のタイトル
			：char *title	作成されたタイトル（既にcom+1分確保されている必要がある）
戻り値　　　：成功	作成されたタイトル
　　　　　　　失敗	NULL
説明　　　　：レスポンスされた回数を含めたタイトルの作成
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *createTitle( char *com )
{

    char	buf[BUFSIZE];		/*作成したタイトル*/
    char	*buf_title;
    char	re1[] = "RE:";
    char	re2[] = "RE";
    int		iresnum;		/*コメント回数*/
    char	strre[BUFSIZE];
    static char title[TITLE_MAXSIZE + 5];

    memset( (char *)title, '\0', sizeof(title) );

    buf_title = malloc(strlen(com) + 5);
    if( buf_title == NULL )
        return NULL;

    if(strcmp(re2,strsub(strre, com,0,2)) == 0) {
        if( strcmp(re1,strsub(strre, com,0,3)) == 0 ) {
            strspl( buf, com, ':' );
            sprintf( buf_title, "RE^2:%s" ,com );
            strcpy( title, buf_title );
        } else {
            strspl( buf, com, ':' );
            iresnum = atoi( strsub(strre, buf, 3, 6) );
            iresnum++;
            sprintf(buf_title, "RE^%d:%s", iresnum, com);
            strcpy( title, buf_title );

        }
    } else {
        sprintf(buf_title,"RE: %s",com);
        if( strlen( buf_title ) > TITLE_MAXSIZE )
            buf_title[TITLE_MAXSIZE] = '\0';
        strcpy( title, buf_title );
    }
    free(buf_title);
    return title;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void printStatus(int, int, int, int, char *, char **)
引数　　　　：int start			表示する始めの記事番号
　　　　　　　int end			表示する最後の記事番号
　　　　　　　int number		最新の記事番号
　　　　　　　int old_number	前回表示した最終番号（既読位置）
　　　　　　　char *type		動作オプション
　　　　　　　char **value		フォームから送信された内容
説明　　　　：掲示板のステータス出力
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void printStatus(int start, int end, int number, int old_number, char *type, char **value, CF config)
{

    int		unread;

    if(old_number != 0) {
        unread = (number - old_number)-(number - start);
        if(unread <= 0)
            unread = 0;
        if(unread <= 0) {
            printf("■ 未読はありません。（◇：未読記事　◆：既読記事）<BR>\n");
        } else {
            printf("■ 未読が<STRONG>%d件</STRONG>あります。（◇：未読記事　◆：既読記事）<BR>\n",unread);
        }
    }
    if(config.flag_write == 1)
        puts("■ 現在、投稿を禁止しています。<BR>\n");
    if(!strcmp(type,"write"))
        printf("■ <STRONG>%sさん</STRONG>からの投稿『<STRONG>%s</STRONG>』を<STRONG>%d番</STRONG>に登録しました。<BR>\n",*(value + 3),*value,number);
    if(!strcmp(type,"del"))
        printf("■ 記事番号<STRONG>%d</STRONG>を削除しました。<BR>\n",atoi(*(value + 1)));
    if(config.tag == 0)
        puts("■ HTMLタグの使用を禁止しています。<BR>\n");
    if(config.regmax != 0)
        printf("■ 最大登録件数は<STRONG>%d件</STRONG>です。これを越えると最も古い記事から自動的に削除されます。<BR>\n",config.regmax);
    if(start > (end + 1))
        printf("■ 現在、<STRONG>%d</STRONG>～<STRONG>%d</STRONG>までの<STRONG>%d件</STRONG>を表\示しています。（削除記事は表\示しません）\n",end+1,start,start-end);
    else if(start == (end + 1))
        printf("■ 現在、記事番号<STRONG>%d</STRONG>を表\示しています。\n",start);
    puts("</FONT>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void printForm2(int, int, int)
引数　　　　：int number		最新の記事番号
　　　　　　　int end			出力する最後の記事番号
　　　　　　　int old_number	前回表示した最終番号（既読位置）
説明　　　　：記事出力終了後のフォーム出力
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void printForm2(int start, int end, int regmax, int max, int old_number, int number)
{

    printf(
        "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0>\n"
        "<TR>\n"
        "	<TD>\n"
        "		<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
        "	</TD>\n"
    );

    if( number < regmax || regmax == 0 ) {
        if( ( start - max ) <= 0 ) {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">\n"
                "	</TD>\n"
            );
        } else {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"次の%d件を表\示\">\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"old\" VALUE=\"%d\">\n"
                "	</TD>\n"
                ,max ,end ,old_number);
        }
    } else {
        if( (start - max) <= (number - regmax) ) {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">\n"
                "	</TD>\n"
            );
        } else {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"次の%d件を表\示\">\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"old\" VALUE=\"%d\">\n"
                "	</TD>\n"
                ,max ,end ,old_number);
        }
    }

    printf(
        "	<TD>\n"
        "		</FORM>\n"
        "	</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        "<HR>\n"
    );

    printf(
        "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0>\n"
        "<TR>\n"
        "	<TD>\n"
        "		<FORM METHOD=POST ACTION=\"mewbbs.cgi\">\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"SUBMIT\" VALUE=\"管理者モード\">\n"
        "	</TD>\n"
        "	<TD WIDTH=5>\n"
        "		　\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=HIDDEN NAME=\"adflag\" VALUE=1>\n"
        "		<INPUT TYPE=\"PASSWORD\" NAME=\"pass\" VALUE=\"\" SIZE=10 MAXLENGTH=%d>\n"
        "		<INPUT TYPE=HIDDEN NAME=\"fadmin\" VALUE=\"\">\n"
        "		<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		</FORM>\n"
        "	</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        , PASSWD_LEN, start);
    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void writeDescription(int, struct tm, char **)
引数　　　　：int number		記事番号
　　　　　　：struct tm *gmt	tm構造体へのポインタ
　　　　　　：char **value		フォームからの送信内容
説明　　　　：記事を登録する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int writeDescription(int number , char **value, CF config)
{

    char	fname[BUFSIZE];					/*ファイル名作成用*/
    char	*remoteaddr;
    char	*remotehost;
    char	*useragent;
    FILE	*fp;						/*記事書き込み*/
    int		i;
    char	buffer[BUFSIZE];
    unsigned int	flag = 0;
    struct tm	*jst;	/*sys/time.hのtm構造体（時間取得）*/
    time_t		ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT→JST*/
    jst = gmtime(&ti);

    remoteaddr = getenv( "REMOTE_ADDR" );
    if( remoteaddr == NULL )
        remoteaddr = empty;

    remotehost = getenv( "REMOTE_HOST" );
    if( remotehost == NULL )
        remotehost = empty;

    useragent = getenv( "HTTP_USER_AGENT" );
    if( useragent == NULL )
        useragent = empty;

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=プロクシ経由の投稿禁止処理-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
    if(config.proxy == 1) {
        if (!ViaProxy ()) {
            fatal_error("■ PROXYサーバからの書き込みは許可されていません。",body);
            return FALSE;
        }
        if (!IsProxy ()) {
            fatal_error("■ PROXYサーバからの書き込みは許可されていません。",body);
            return FALSE;
        }
    } else if(config.proxy == 2) {
        if(!isAnonymousProxy()) {
            fatal_error("■ 匿名PROXYサーバからの書き込みは許可されていません。",body);
            return FALSE;
        }
    }
    /*-=-=-=-=-=-=-=-=-=-=-=-=指定したホストからの投稿禁止処理-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if((fp = fopen(DENIED_LIST, "r")) == NULL) {
        fatal_error("■ 投稿禁止ホストリスト(denied)が存在しないか、読み込み権限が与えられていません。",body);
        return FALSE;
    }

    for(i = 0;  ; i++) {
        if(fgets(buffer,BUFSIZE,fp) == NULL)
            break;
        removeNewline(buffer);
        if(strstr(remoteaddr, buffer) != NULL ) {
            fatal_error("■ このホストからの投稿は禁止されています。",body);
            return FALSE;
        } else if(strstr(remotehost, buffer) != NULL ) {
            fatal_error("■ このホストからの投稿は禁止されています。",body);
            return FALSE;
        }
    }
    fclose(fp);

    /*-=-=-=-=-=-=-=-=-=-=-=-=ホスト名から逆引きして偽ホストからの投稿禁止処理-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if( config.hostchk == 1 ) {
        if (!ReverseDNS()) {
            fatal_error("■ 不正なホストからの投稿は禁止されています。", "<BODY>");
            return FALSE;
        }
    }

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=連続投稿禁止処理-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if((number - 1) > 1) {
        sprintf(fname,"./file/%d",number - 1);
        if((fp = fopen(fname,"r")) != NULL) {

            for(i = 0; i < 6; i++) {
                if(fgets(buffer,BUFSIZE,fp) == NULL) {
                    if(ferror(fp)) {
                        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 152)",body);
                        return FALSE;
                    }
                    if(feof(fp))
                        break;
                }
                removeNewline(buffer);
                if( i == 2 ) {
                    if(strcmp( buffer, *(value + 3) ) != 0)		/*投稿者比較*/
                        flag = 1;
                } else if( i == 3 ) {
                    if(strcmp( buffer, *(value + 4) ) != 0)		/*メールアドレス比較*/
                        flag = 1;
                } else if( i == 5 ) {
                    if(strcmp( buffer, *value ) != 0)			/*タイトル比較*/
                        flag = 1;
                } else if( i == 6 ) {								/*REMOTE_ADDR比較*/
                    if(strcmp( buffer, remoteaddr ) != 0)
                        flag = 1;
                }
            }
            fclose(fp);
            if( flag == 0 ) {
                fatal_error("■ 同じ投稿者から同じ内容を連続して投稿できません。",body);
                return FALSE;
            }
        }
    }

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    sprintf(fname,"./file/%d",number);	/*pathを含めたファイル名作成*/
    sprintf(buffer, "./file/%dcp", number);

    if(access(fname, 00) == 0)
        rename(fname, buffer);

    if((fp = fopen(fname,"w")) == NULL) {
        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 153)",body);
        return FALSE;
    }

    /*記事内容をフォーマットに従って書き込む*/
    fprintf(fp,"%05d\n",number);		/*[0]記事番号*/
    fprintf(fp,"%02d月%02d日(%s)%02d時%02d分\n",jst->tm_mon+1,jst->tm_mday,weekday[jst->tm_wday],jst->tm_hour,jst->tm_min);/*[1]投稿日時*/
    fprintf(fp,"%s\n",*(value + 3));	/*[2]投稿者*/
    fprintf(fp,"%s\n",*(value + 4));	/*[3]メールアドレス*/
    fprintf(fp,"%s\n",*(value + 8));	/*[4]未使用*/
    fprintf(fp,"%s\n",*value);			/*[5]タイトル*/
    fprintf(fp,"%s\n",useragent);		/*[6]ユーザーエージェント*/
    fprintf(fp,"%s\n",remoteaddr);		/*[7]リモートホスト*/
    fprintf(fp,"%s\n",*(value + 6));	/*[8]コメント元番号*/
    fprintf(fp,"%s\n",*(value + 7));	/*[9]コメント元投稿者*/
    fprintf(fp,"0\n");					/*[10]記事属性（ここは通常記事なので0）*/
    fprintf(fp,"%s\n",*(value + 2));	/*[11]URL*/
    fprintf(fp,"\n\n\n");				/*[12-14]残りの未使用領域の出力（改行のみ）*/
    fprintf(fp,"%s",*(value + 1));		/*[15]コメント*/
    fclose(fp);

    return TRUE;
}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void limitDescription(int)
引数　　　　：int number	最新の記事番号
説明　　　　：登録制限を越えた記事の削除
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*--- 登録数制限を越えた記事の削除 ---*/
void limitDescription(int number, int regmax)
{

    char	fname[BUFSIZE];
    int		i;			/*カウンタ*/

    if(regmax != 0 && regmax < number) {
        for(i = (number - regmax); ; i--) {
            sprintf(fname,"./file/%d",i);
            if(unlink(fname) != 0)
                break;
        }
    }
    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void deleteDescription(int, char **, struct tm)
引数　　　　：int count			削除の対象となる記事数
　　　　　　　char **name		フォームで送られてきた削除対象記事番号
　　　　　　　struct tm *jst	tm構造体へのポインタ
説明　　　　：記事を削除し、バックアップファイルを作成する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int deleteDescription(int count, char **name )
{

    int		i,j;				/*カウンタ*/
    int		delnumber;			/*削除対象記事番号*/
    char	buffer[BUFSIZE];	/*ファイル読み込みバッファ*/
    char	fname[BUFSIZE];		/*ファイル名作成用*/
    FILE	*fp;				/*削除記事を読み込む*/
    FILE	*fpbackup;			/*バックアップファイルに書き込む*/
    struct tm	*jst;	/*sys/time.hのtm構造体（時間取得）*/
    time_t		ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT→JST*/
    jst = gmtime(&ti);

    if(count < 2) {
        fatal_error("■ 削除する記事を選択してください。", body);
        return FALSE;
    }

    for(i = 0; i < (count - 1) ; i++) {
        delnumber = atoi(*(name + i));
        if(delnumber == 0)
            continue;
        sprintf(fname, "./file/%d", delnumber);		/*pathを含めた削除対象ファイル名作成*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("■ 削除先の番号が指定されていないか、該当する記事はありません。", body);
            return FALSE;
        } else {
            for(j = 0;  ; j++) {
                if(fgets(buffer, BUFSIZE, fp) == NULL) {
                    if(ferror(fp)) {
                        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 170)", body);
                        return FALSE;
                    }
                    if(feof(fp))
                        break;
                }
                if(j == 10)
                    break;
            }
            if(atoi(buffer) == 1 || atoi(buffer) == 2) {	/*削除記事*/
                fclose(fp);
                continue;
            }
        }
        fclose(fp);

        sprintf(fname, "./file/%d", delnumber);		/*pathを含めた削除対象ファイル名作成*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("■ 削除先の番号が指定されていないか、該当する記事はありません。", body);
            return FALSE;
        }
        sprintf(fname, "./file/%ddel", delnumber);	/*pathを含めたバックアップファイル名作成*/
        if((fpbackup = fopen(fname, "w")) == NULL) {
            fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 171)", body);
            return FALSE;
        }
        /*--- 削除対象ファイルを読み込み、バックアップファイルに書き込む ---*/
        for(j = 0;  ; j++) {
            if(fgets(buffer, BUFSIZE, fp) == NULL) {
                if(ferror(fp)) {
                    fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 172)", body);
                    return FALSE;
                }
                if(feof(fp))
                    break;
            }
            fputs(buffer,fpbackup);
        }
        fclose(fp);
        fclose(fpbackup);

        /*--- 削除記事のフォーマット通りに出力 ---*/
        sprintf(fname,"./file/%d",delnumber);
        if((fp = fopen(fname,"w")) == NULL) {
            fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 173)", body);
            return FALSE;
        }
        fprintf(fp,"%05d\n",delnumber);
        fprintf(fp,"%02d月%02d日(%s)%02d時%02d分\n",jst->tm_mon+1,jst->tm_mday,weekday[jst->tm_wday],jst->tm_hour,jst->tm_min);
        fputs("管理者削除\n",fp);
        fputs("\n",fp);/*メールアドレス*/
        fputs("\n",fp);/*コメント回数*/
        fputs("管理者削除\n",fp);
        fputs("\n",fp);/*HTTP_USER_AGENT*/
        fputs("\n",fp);/*REMOTE_HOST*/
        fputs("･････\n",fp);/*コメント元番号*/
        fputs("-----\n",fp);/*コメント元投稿者*/
        fputs("2\n",fp);/*管理者削除*/
        fputs("\n\n\n\n\n",fp);
        fclose(fp);
    }
    return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void undelDescription(int, char **)
引数　　　　：int count		削除の対象となる記事数
　　　　　　：char **name	フォームで送られてきた削除対象記事番号
説明　　　　：削除記事を復活し、バックアップファイルを削除する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int undelDescription(int count, char **name)
{

    int		i,j;				/*カウンタ*/
    int		undelnumber;		/*復活対象削除記事*/
    char	buffer[BUFSIZE];	/*ファイル読み込みバッファ*/
    char	fname[BUFSIZE];			/*ファイル名作成用*/
    FILE	*fp;				/*削除記事を読み込む*/
    FILE	*fpbackup;			/*バックアップファイルに書き込む*/

    if(count < 2) {
        fatal_error("■ 復活させる削除記事を選択してください。", body);
        return FALSE;
    }

    for(i = 0; i < (count - 1) ; i++) {
        undelnumber = atoi(*(name + i));
        if(undelnumber == 0)
            continue;
        sprintf(fname, "./file/%d", undelnumber);	/*pathを含めたバックアップファイル名作成*/
        if((fpbackup = fopen(fname, "w")) == NULL) {
            fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 180)", body);
            return FALSE;
        }
        sprintf(fname, "./file/%ddel", undelnumber);		/*pathを含めた削除対象ファイル名作成*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("■ 復活記事の番号が指定されていないか、該当する記事はありません。", body);
            return FALSE;
        }
        /*--- 削除対象ファイルを読み込み、バックアップファイルに書き込む ---*/
        for(j = 0;  ; j++) {
            if(fgets(buffer, BUFSIZE, fp) == NULL) {
                if(ferror(fp)) {
                    fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 181)", body);
                    return FALSE;
                }
                if(feof(fp))
                    break;
            }
            fputs(buffer,fpbackup);
        }
        fclose(fp);
        fclose(fpbackup);
        unlink(fname);
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void quoteDescription(int)
引数　　　　：int resnumber		引用する対象の記事番号
説明　　　　：該当する記事を引用し、出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int quoteDescription(int resnumber)
{

    int		i;
    char	fname[FNAME_LEN];
    char	**file2Dim = NULL;

    sprintf(fname,"./file/%d",resnumber);
    file2Dim = readFile( fname );
    if( file2Dim == NULL ) {
        fatal_error("■ コメント先の番号が指定されていないか、該当する記事はありません。", body);
        return FALSE;
    }

    for( i = 0; *(file2Dim + i); i++ ) {
        if(i >= 15)
            printf(">%s", *(file2Dim + i));
    }

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void correctDescription(char **)
引数　　　　：char **value		フォームから送信した修正後の内容
説明　　　　；フォームから送信された修正後の内容を該当する記事番号に書き込む。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int correctDescription(char **value)
{

    int		i;
    char	fname[BUFSIZE];					/*ファイル名作成用*/
    FILE	*fp;						/*記事書き込み*/

    sprintf(fname,"./file/%d",atoi(*value));	/*pathを含めたファイル名作成*/

    if((fp = fopen(fname,"w")) == NULL) {
        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 200)", body);
        return FALSE;
    }
    /*記事内容を記事ファイルフォーマットに従って書き込む*/
    for(i = 0; i < 12; i++)
        fprintf(fp,"%s\n",*(value + i));/*[0-11]*/
    fprintf(fp,"\n\n\n");				/*[12-14]残りの未使用領域の出力（改行のみ）*/
    fprintf(fp,"%s",*(value + 12));		/*[15]コメント*/
    fclose(fp);

    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int printDescription(int, FILE *, int)
引数　　　　：int number			表示する記事番号
　　　　　　：FILE *fp			nowへのファイルポインタ
　　　　　　：int old_number	前回表示した最終番号（既読位置）
説明　　　　：numberで指定された記事を出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int printDescription(int number, FILE *fp, int old_number, CF config)
{

    int		i;
    unsigned int	brouser;
    char	buffer[BUFSIZE];
    char	comment[15][BUFSIZE];	/*情報行読み込み*/

    brouser = selectBrouser();

    for(i = 0; i < 15; i++) {
        if(fgets(comment[i],BUFSIZE,fp) == NULL)
            break;
        removeNewline(comment[i]);
        if(i == 10) {
            if(atoi(comment[i]) != 0)
                return FALSE;
        }
    }
    printf(
        "<!-- 記事番号%05d開始 -->\n"
        "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH=\"100%%\">\n"
        "<TR BGCOLOR=\"%s\">\n"
        "	<TD NOWRAP>\n"
        ,number, config.title_color);
    if(old_number != 0) {
        if(number > old_number) {
            printf("		 %s◇ %04d　%s\n　",font,number,comment[1]);
        } else {
            printf("		 %s◆ %04d　%s\n　",font,number,comment[1]);
        }
    } else {
        printf("		 %s%04d　%s\n　",font,number,comment[1]);
    }

    if(strstr(*(comment + 3),"@") != NULL)	/*メールドレスに@が含まれている場合*/
        printf("<A HREF=\"mailto:%s\"><STRONG>%s</STRONG></A>\n",comment[3],comment[2]);
    else
        printf("<STRONG>%s</STRONG>\n",comment[2]);

    printf("　　<STRONG>%s</STRONG>　　\n",comment[5]);
    printf(
        "	</TD>\n"
        "	<TD NOWRAP WIDTH=\"100%%\">\n"
        "		<FORM METHOD=POST ACTION=\"./mewbbs.cgi\">\n"
        "		　<INPUT TYPE=\"IMAGE\" SRC=\"./gif/res.gif\" ALT=\"#%dにコメントする\" BORDER=0>\n"
        ,number);

    if(atoi(comment[8]) != 0) {
        printf(
            "		<A HREF=\"./viewer.cgi?number=%d\" TARGET=\"_new\"><IMG SRC=\"./gif/fw.gif\" BORDER=0 ALT=\"返信元記事(#%d)を表\示\"></A>\n"
            , atoi(*(comment + 8)), atoi(*(comment + 8)));
    }

    printf(
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=HIDDEN NAME=\"res\" VALUE=\"1\">\n"
        "		<INPUT TYPE=HIDDEN NAME=\"num\" VALUE=\"%d\">\n"
        "		</FORM>\n"
        "	</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        "<BLOCKQUOTE>"
        ,number);
    if( brouser == 1 )
        printf("<BIG>");

    printf("<PRE>%s", font);

    for(i = 0; ; i++) {
        if(fgets(buffer,BUFSIZE,fp) == NULL)
            break;
        if(*buffer == '>')
            printf("<I>%s</I>",buffer);
        else
            fputs(buffer,stdout);
    }
    printUrl(*(comment + 11));	/*URL出力*/

    printf(
        "</PRE>\n"
        "</BIG>\n"
        "</BLOCKQUOTE>\n"
        "<!--Remote Host: %s-->\n"
        "<!-- 記事番号%05d終了 -->\n\n"
        "<HR>\n\n"
        ,*(comment + 7) ,number);

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formConfirm(char **, char **)
引数　　　　：char **name	送信されたフォームのNAME
　　　　　　：char **value	送信されたフォームのVALUE
説明　　　　：投稿内容を確認する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formConfirm(char **name, char **value, int count)
{

    int		i;		/*カウンタ用*/

    /*--- 確認画面出力 ---*/
    printPageHeader("登録内容の確認");
    puts(body);
    puts("<BR><TABLE BORDER=2 CELLPADDING=3 WIDTH=98%>");
    printf("<TR><TH COLSPAN=2>登録内容の確認</TH></TR>");
    printf("<TR><TD WIDTH=20%% NOWRAP>題名</TD><TD WIDTH=80%% NOWRAP>%s</TD></TR>",*value);

    if( strlen( *(value + 1) ) >= 1 )
        printf("<TR><TD VALIGN=TOP>内容</TD><TD><FONT SIZE=+1><PRE>%s</TD></TR>",*(value + 1));

    if(check_url(*(value + 2)) == 1)	/*URLの入力がある*/
        printf("<TR><TD>参照アドレス</TD><TD>%s</TD></TR>",*(value + 2));

    printf("<TR><TD>投稿者</TD><TD>%s</TD></TR>",*(value + 3));
    printf("<TR><TD>e-mail</TD><TD>%s</TD></TR>",*(value + 4));
    puts("</TABLE><FORM ACTION=\"mewbbs.cgi\" METHOD=\"POST\">");

    for(i = 0; *(name + i); i++) {
        if(strcmp("confirm", *(name + i)) && strcmp("write", *(name + i)))
            printf("<INPUT TYPE=\"HIDDEN\" NAME=\"%s\" VALUE=\"%s\">" ,*(name + i) ,*(value + i));
    }
    puts("<INPUT TYPE=\"HIDDEN\" NAME=\"confirm\" VALUE=\"1\">");
    puts("</CENTER><INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"　　登　　録　　\">");
    puts("</FORM>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formAdmin(char *)
引数　　　　：char *qspasswd		フォームに入力されたパスワード
説明　　　　：管理者モードのフォームを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formAdmin(char *qspasswd)
{
    DPRINTL(getenv("REMOTE_ADDR"));
    if(strcmp(master,qspasswd) != 0) {
        fatal_error("■ パスワードが間違っています。もう一度、お確かめください。", body);
        return FALSE;
    } else {
        printPageHeader("管理者モード");
        puts(body);
        /*--- タイトル表示 ---*/
        puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");

        puts("<BR><BR>■ 管理者モードに移行しました。行いたい作業を選択してください。<BR><BR>");
        puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>"
             "<TR><TD>"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fdel\" VALUE=\"記事削除\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fundel\" VALUE=\"記事復活\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fcorselect\" VALUE=\"記事修正\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fchcfg\" VALUE=\"設定変更\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fchpass\" VALUE=\"パスワード変更\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fsecurity\" VALUE=\"セキュリティ設定\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"通常モード\">");
        printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
        puts("</TD></TR></TABLE></BODY></HTML>");
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formDelete(void)
説明　　　　：現在の記事番号をスタートととして、削除用タイトルを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formDelete(int max)
{

    char	fname[BUFSIZE];			/*ファイル名作成*/
    char	*comment[15];		/*内容表示用作業領域*/
    int		i,j,k;				/*カウンタ*/
    char	buffer[BUFSIZE];	/*バッファ*/
    FILE	*fpcomment;

    printPageHeader("記事削除");
    puts(body);
    /*--- タイトル表示 ---*/
    puts("<BR><BR>■ 記事の削除を行います。削除したい記事をチェックし、「削除」を押してください。");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>");
    printf("<TR><TH WIDTH=1%% NOWRAP>　</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>番号");
    puts("<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>投稿日時"
         "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>投稿者"
         "<TH ALIGN=\"CENTER\">タイトル");
    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%d",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*改行を取り除く*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 240)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }
        /*--- 通常記事（新規・コメント） ---*/
        if(atoi(*(comment + 10)) == 0) {
            if(atoi(*(comment + 8)) == 0) {	/*新規記事*/
                printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            } else {	/*コメント記事*/
                printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s"
                       "<TD NOWRAP>%s"
                       "<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            }
        } else if(atoi(*(comment + 10)) == 1) {	/*投稿者削除*/
            printf("<TR><TD>　");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        } else if(atoi(*(comment + 10)) == 2) {	/*管理者削除*/
            printf("<TR><TD>　");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        }
        fclose(fpcomment);
        /*--- 作業領域解放 ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"del\" VALUE=\"　削　除　\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formUndelete(void)
説明　　　　；現在の記事番号をスタートととして、記事復活用タイトルを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formUndelete(int max)
{

    char	fname[BUFSIZE];			/*ファイル名作成*/
    char	*comment[15];		/*内容表示用作業領域*/
    int		i,j,k;				/*カウンタ*/
    char	buffer[BUFSIZE];	/*バッファ*/
    FILE	*fpcomment;

    printPageHeader("記事復活");
    puts(body);
    /*--- タイトル表示 ---*/
    puts("<BR><BR>■ 削除された記事の復活を行います。復活したい記事をチェックし、「復活」を押してください。<BR>");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>");
    printf("<TR><TH WIDTH=1%% NOWRAP>　</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>番号");
    puts("<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>投稿日時"
         "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>投稿者"
         "<TH ALIGN=\"CENTER\">タイトル");
    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%ddel",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*改行を取り除く*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 250)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }

        printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
        printf("<TD NOWRAP>%s"
               "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));

        fclose(fpcomment);
        /*--- 作業領域解放 ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"undel\" VALUE=\"　復　活　\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formCorSelect(void)
説明　　　　；現在の記事番号をスタートととして、記事修正用タイトルを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formCorSelect(int max)
{

    char	fname[BUFSIZE];			/*ファイル名作成*/
    char	*comment[15];		/*内容表示用作業領域*/
    int		i,j,k;				/*カウンタ*/
    char	buffer[BUFSIZE];	/*バッファ*/
    FILE	*fpcomment;

    printPageHeader("記事修正");
    puts(body);

    /*--- タイトル表示 ---*/
    printf("<BR><BR>\n"
           "■ 記事の修正を行います。修正したい記事をチェックし、「修正」を押してください。<BR>\n"
           "<FORM METHOD=POST ACTION=\"mewbbs.cgi\">\n"
           "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>"
           "<TR><TH WIDTH=1%% NOWRAP>　</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>番号"
           "<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>投稿日時"
           "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>投稿者"
           "<TH ALIGN=\"CENTER\">タイトル");

    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%d",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*改行を取り除く*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 260)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }
        /*--- 通常記事（新規・コメント） ---*/
        if(atoi(*(comment + 10)) == 0) {
            if(atoi(*(comment + 8)) == 0) {	/*新規記事*/
                printf("<TR><TD><INPUT TYPE=RADIO NAME=num VALUE=%s>",*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            } else {	/*コメント記事*/
                printf("<TR><TD><INPUT TYPE=RADIO NAME=num VALUE=%s>",*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s"
                       "<TD NOWRAP>%s"
                       "<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            }
        } else if(atoi(*(comment + 10)) == 1) {	/*投稿者削除*/
            printf("<TR><TD>　");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        } else if(atoi(*(comment + 10)) == 2) {	/*管理者削除*/
            printf("<TR><TD>　");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        }
        fclose(fpcomment);
        /*--- 作業領域解放 ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"fcorinput\" VALUE=\"　修　正　\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formCorInput(int)
引数　　　　：int resnumber		修正対象の記事番号
説明　　　　；resnumberで指定された番号の修正用フォームを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formCorInput(int resnumber)
{

    int		i;
    char	**comment = NULL;
    char	fname[BUFSIZE];

    if (resnumber == 0) {
        fatal_error("■ 内容修正する記事を選択してください。", body);
        return FALSE;
    } else if(resnumber > 0) {
        sprintf(fname,"./file/%d",resnumber);
        comment = readFile( fname );
        if( comment == NULL ) {
            return FALSE;
        } else {
            for(i = 0; *(comment + i) && i < 15; i++)
                removeNewline( *(comment + i) );	/*改行を取り除く*/
        }
    }
    printPageHeader("指定番号の記事修正");
    puts(body);
    printf("<BR><BR>■ %d番の修正を行います。記事を修正し「修正」を押してください。<BR>",resnumber);

    puts("<FORM METHOD=\"POST\" ACTION=\"mewbbs.cgi\">\n<TABLE BORDER=1>\n");

    printf("<INPUT TYPE=HIDDEN NAME=number VALUE=\"%s\">",*comment);
    printf("<INPUT TYPE=HIDDEN NAME=time VALUE=\"%s\">",*(comment + 1));
    puts("<TR><TD><STRONG>投稿者</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=writer VALUE=\"%s\" SIZE=20 MAXLENGTH=30></TD></TR>",*(comment + 2));
    puts("<TR><TD><STRONG>e-mail</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=mail VALUE=\"%s\" SIZE=20 MAXLENGTH=150></TD></TR>",*(comment + 3));
    printf("<INPUT TYPE=HIDDEN NAME=comcount VALUE=\"%s\">",*(comment + 4));
    puts("<TR><TD><STRONG>タイトル</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=title VALUE=\"%s\" SIZE=40 MAXLENGTH=40></TD></TR>",*(comment + 5));
    printf("<INPUT TYPE=HIDDEN NAME=useragent VALUE=\"%s\">",*(comment + 6));
    printf("<INPUT TYPE=HIDDEN NAME=remotehost VALUE=\"%s\">",*(comment + 7));
    printf("<INPUT TYPE=HIDDEN NAME=comnumber VALUE=\"%s\">",*(comment + 8));
    printf("<INPUT TYPE=HIDDEN NAME=comwriter VALUE=\"%s\">",*(comment + 9));
    printf("<INPUT TYPE=HIDDEN NAME=atribute VALUE=\"%s\">",*(comment + 10));
    puts("<TR><TD><STRONG>参照アドレス</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=url VALUE=\"%s\" SIZE=60 MAXLENGTH=300></TD></TR>",*(comment + 11));
    puts("<TR><TD VALIGN=\"TOP\"><STRONG>内容</STRONG></TD>");
    puts("<TD><TEXTAREA NAME=\"comment\" ROWS=8 COLS=60>");
    for(i = 15; *(comment + i); i++)
        fputs(*(comment + i),stdout);

    printf(
        "</TEXTAREA></TD></TR></TABLE>"
        "<TABLE BORDER=1>"
        "<TR><TD COLSPAN=2><INPUT TYPE=\"SUBMIT\" NAME=\"correction\" VALUE=\"　修　正　\">"
    );
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    puts("<INPUT TYPE=\"SUBMIT\" NAME=\"fcorselect\" VALUE=\"　戻　る　\">");
    puts("</TD></TR></TABLE>");

    freeTwoDimArray( comment );

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formConfig(void)
説明　　　　；設定変更の為のフォームを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formConfig(CF config)
{

    printPageHeader("設定変更");
    puts(body);
    printf("<BR><BR>■ 設定の変更を行います。変更したい項目を入力し「変更」を押してください。（<A HREF=\"%s#CONFIG\" TARGET=\"_new\">ヘルプ</A>）<BR>", ADMIN_HELP_URL);
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>");
    printf("<TR><TD NOWRAP><STRONG>ウインドウタイトル</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"APTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.aptitle);
    printf("<TR><TD NOWRAP><STRONG>メインタイトル</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"MAINTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.maintitle);
    printf("<TR><TD NOWRAP><STRONG>サブタイトル</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"SUBTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.subtitle);
    printf("<TR><TD NOWRAP><STRONG>表\示記事数(1-50)</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"max\" VALUE=\"%d\" SIZE=2 MAXLENGTH=2></TD></TR>",config.max);
    if(config.tag == 1)
        printf("<TR><TD NOWRAP><STRONG>HTMLタグ</STRONG></TD><TD><SELECT NAME=\"TAG\"><OPTION VALUE=1>○<OPTION VALUE=0>×</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>HTMLタグ</STRONG></TD><TD><SELECT NAME=\"TAG\"><OPTION VALUE=0>×<OPTION VALUE=1>○</SELECT></TD></TR>");

    if(config.flag_write == 0)
        printf("<TR><TD NOWRAP><STRONG>書き込み許可</STRONG></TD><TD><SELECT NAME=\"WRITE\"><OPTION VALUE=0>○<OPTION VALUE=1>×</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>書き込み許可</STRONG></TD><TD><SELECT NAME=\"WRITE\"><OPTION VALUE=1>×<OPTION VALUE=0>○</SELECT></TD></TR>");

    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>背景</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"BACKGROUND\" VALUE=\"%s\" SIZE=50 MAXLENGTH=200></TD></TR>",CC_URL ,config.background);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>背景色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"BGCOLOR\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL ,config.bgcolor);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>文字色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"TEXT\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.text);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>リンク文字色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"LINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.link_color);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>既リンク文字色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"VLINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.vlink);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>リンク中文字色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"ALINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.alink);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>記事タイトル背景色</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"TCOLOR\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.title_color);
    if(config.fweight == 0)
        printf("<TR><TD NOWRAP><STRONG>文字の太さ</STRONG></TD><TD><SELECT NAME=\"FWEIGHT\"><OPTION VALUE=0>通常<OPTION VALUE=1>太い</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>文字の太さ</STRONG></TD><TD><SELECT NAME=\"FWEIGHT\"><OPTION VALUE=1>太い<OPTION VALUE=0>通常</SELECT></TD></TR>");

    if(config.fsize == 2)
        printf("<TR><TD NOWRAP><STRONG>文字の大きさ</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=2>小さい<OPTION VALUE=3>通常<OPTION VALUE=4>大きい</SELECT></TD></TR>");
    else if(config.fsize == 4)
        printf("<TR><TD NOWRAP><STRONG>文字の大きさ</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=4>大きい<OPTION VALUE=3>通常<OPTION VALUE=2>小さい</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>文字の大きさ</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=3>通常<OPTION VALUE=2>小さい<OPTION VALUE=4>大きい</SELECT></TD></TR>");

    printf("<TR><TD NOWRAP><STRONG>最大登録数</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"REGMAX\" VALUE=\"%d\" SIZE=5 MAXLENGTH=10></TD></TR>",config.regmax);
    printf("<TR><TD NOWRAP><STRONG>ホームページ</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"HOME_URL\" VALUE=\"%s\" SIZE=50 MAXLENGTH=200></TD></TR>",config.home_url);

    puts("<TR><TD COLSPAN=2 NOWRAP><INPUT TYPE=\"SUBMIT\" NAME=\"chcfg\" VALUE=\"　変　更　\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formChpass(void)
説明　　　　；パスワード変更用のフォームを出力する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formChpass(void)
{

    printPageHeader("パスワード変更");
    puts(body);
    printf(
        "<BR><BR>\n"
        "■ パスワードの変更を行います。変更するパスワードを入力し、「変更」を押してください。\n"
        "<BR><BR>\n"
        "<FORM METHOD=POST ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>\n"
        "<TR>\n"
        "	<TD NOWRAP>\n"
        "		<STRONG>新パスワード</STRONG>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"PASSWORD\" NAME=\"NEW_PASS\" VALUE=\"\" SIZE=8 MAXLENGTH=8>\n"
        "	</TD>\n"
        "<TR>\n"
        "	<TD NOWRAP>\n"
        "		<STRONG>確認</STRONG>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"PASSWORD\" NAME=\"CF_PASS\" VALUE=\"\" SIZE=8 MAXLENGTH=8>\n"
        "	</TD>\n"
        "</TR>\n"
        "<TR>\n"
        "	<TD COLSPAN=2>\n"
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"chpass\" VALUE=\"　変　更　\">"
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">"
        "	</TD>\n"
        "	</TR>\n"
        "</TABLE>\n"
    );
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    printf(
        "</FORM>\n"
        "</BODY>\n"
        "</HTML>\n"
    );

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void formSecurity(void)
説明　　　　；.deniedから投稿禁止ホストを読み込み表示する。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formSecurity(int proxy, int hostchk)
{

    int		i;				/*カウンタ*/
    char	buffer[BUFSIZE];	/*バッファ*/
    FILE	*fp;

    printPageHeader("セキュリティ設定");
    puts(body);

    printf(
        "<!-- セキュリティ関連の設定変更フォーム -->\n"
        "<BR><BR>\n"
        "■ セキュリティ関連の設定を行います。（<A HREF=\"%s#SECURITY\" TARGET=\"_new\">ヘルプ</A>）", ADMIN_HELP_URL);
    printf(
        "<BR>\n<HR>\n<BR>\n"
        "<BLOCKQUOTE>\n"
        "○ 投稿禁止ホストの追加を行います。\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>\n	"
        "<TR>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"TEXT\" NAME=\"host\" SIZE=50 MAXLENGTH=1000>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"HIDDEN\" NAME=\"addhost\" VALUE=\"addhost\">\n"
    );
    printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n" ,now);
    printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"pass\" VALUE=\"%s\">\n" ,master);
    printf(
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"\" VALUE=\"　追　加　\">\n"
        "	</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        "</FORM>\n"
        "</BLOCKQUOTE>\n"
        "<HR>\n"
    );


    /*DENIEDリストの読み込みと表示*/
    if((fp = fopen(DENIED_LIST,"r")) == NULL) {
        fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 301)", body);
        return FALSE;
    } else {
        if(fgets(buffer,BUFSIZE,fp) != NULL) {

            rewind( fp );

            printf(
                "<BR>\n"
                "<BLOCKQUOTE>\n"
                "○ 投稿禁止ホストの削除を行います。\n"
                "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
                "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=\"50%%\">\n"
            );

            for(i = 1; ; i++) {
                if(fgets(buffer,BUFSIZE,fp) == NULL)
                    break;
                if(strlen(buffer) > 1) {
                    removeNewline(buffer);	/*改行を取り除く*/
                    printf("<TR><TD><INPUT TYPE=\"CHECKBOX\" NAME=\"__%d\" VALUE=\"%d\" WIDTH=\"1%%\"></TD>"
                           "<TD NOWRAP WIDTH=\"99%%\">%s</TD></TR>",i,i,buffer);
                }
            }
            fclose(fp);

            printf(
                "</TD>\n"
                "</TR>\n"
                "</TABLE>\n"
                "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>\n"
                "<TR>\n"
                "<TD>\n"
                "	<INPUT TYPE=\"SUBMIT\" NAME=\"delhosts\" VALUE=\"　削　除　\">\n"
            );
            printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">",now);
            printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"pass\" VALUE=\"%s\">",master);
            printf(
                "</TD>\n"
                "</TR>\n"
                "</TABLE>\n"
                "</FORM>\n"
                "</BLOCKQUOTE>\n"
                "<HR>\n"
            );
        }

    }

    printf(
        "<!-- その他のセキュリティ設定変更フォーム -->\n"
        "<BR>\n"
        "<BLOCKQUOTE>\n"
        "○ その他のセキュリティ設定を行います。\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=3 WIDTH=\"80%%\">\n"
    );

    printf(
        "<TR>\n"
        "<TD NOWRAP WIDTH=\"40%%\">\n"
        "	・PROXY経由投稿"
        "</TD>"
    );
    if(proxy == 1) {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=1>PROXY経由の投稿禁止</OPTION>\n"
            "	<OPTION VALUE=0>全てのホストから投稿許可</OPTION>\n"
            "	<OPTION VALUE=2>匿名PROXY経由の投稿のみ禁止</OPTION>\n"
            "	</SELECT>\n"
            "</TD>\n"
        );
    } else if(proxy == 2) {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=2>匿名PROXY経由の投稿のみ禁止</OPTION>\n"
            "	<OPTION VALUE=0>全てのホストから投稿許可</OPTION>\n"
            "	<OPTION VALUE=1>PROXY経由の投稿禁止</OPTION>\n"
            "	</SELECT>\n"
            "</TD>\n"
        );
    } else {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=0>全てのホストから投稿許可</OPTION>\n"
            "	<OPTION VALUE=1>PROXY経由の投稿禁止</OPTION>\n"
            "	<OPTION VALUE=2>匿名PROXY経由の投稿のみ禁止</OPTION>\n"
            "	</SELECT>\n"
            "</TD>\n"
        );
    }
    printf(
        "</TR>\n"
    );
    printf(
        "<TR>\n"
        "<TD NOWRAP>\n"
        "	・不正なホストからの投稿"
        "</TD>"
    );
    if(hostchk == 1) {
        printf(
            "<TD NOWRAP>\n"
            "	<INPUT TYPE=\"CHECKBOX\" NAME=\"HOSTCHK\" VALUE=1 CHECKED>\n"
            "	拒否する\n"
            "</TD>\n"
        );
    } else {
        printf(
            "<TD NOWRAP>\n"
            "	<INPUT TYPE=\"CHECKBOX\" NAME=\"HOSTCHK\" VALUE=1>\n"
            "	拒否する\n"
            "</TD>\n"
        );
    }
    printf(
        "</TR>\n"
    );

    printf(
        "<TR>\n"
        "<TD NOWRAP COLSPAN=2>\n"
        "	<BR>\n"
        "	<INPUT TYPE=\"SUBMIT\" NAME=\"chsec\" VALUE=\"　変　更　\">\n"
    );
    printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n",now);
    printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"pass\" VALUE=\"%s\">\n",master);
    printf("</TD>\n");
    printf(
        "</TR>\n"
        "</TABLE>\n"
        "</FORM>\n"
        "</BLOCKQUOTE>\n"
        "<HR>\n"
    );

    printf(
        "<BR>\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>\n"
        "<TR>\n"
        "<TD>\n"
        "	<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">\n"
    );
    printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n" ,now);
    printf("	<INPUT TYPE=\"HIDDEN\" NAME=\"pass\" VALUE=\"%s\">\n" ,master);
    printf(
        "</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        "</FORM>\n"
        "</BODY>\n"
        "</HTML>\n"
    );

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void chPass(char **value)
引数　　　　：char **value		フォームから送られてきたパスワード
説明　　　　；フォームから送信された新パスワードを確認パスワードを認証し、
　　　　　　　正しければ、新パスワードを暗号化しpasswdに書き込む。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int chPass(char **value)
{

    FILE *fp;
    char *passwd;

    if(strcmp(*value, *(value + 1))) {
        fatal_error("■ 新パスワードと確認パスワードの入力が異なっています。もう一度お確かめください。", body);
        return FALSE;
    }
    if((fp = fopen(PASSWD_PATH,"w")) == NULL) {
        fatal_error("■ パスワードファイルが存在しないか、otherに書き込み権限が与えられていません。", body);
        return FALSE;
    }
    passwd = crypt(*value,SALT);
    fprintf(fp,"%s",passwd);
    fclose(fp);
    printPageHeader("パスワード変更完了！");
    puts(body);
    puts("<BR><BR>■ パスワードが変更されました。パスワードの管理には十\分注意してください。");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\"><INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"　戻　る　\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\"></TD></TR></TABLE></BODY></HTML>",passwd);

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void getPass(void)
説明　　　　；passwdからパスワードを取得し、暗号化してmasterにセットする。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getPass(void)
{

    FILE *fp;

    if((fp = fopen(PASSWD_PATH,"r")) == NULL) {
        fatal_error("■ パスワードファイルが存在しないか、otherに読み込み権限が与えられていません。", body);
        return FALSE;
    } else {
        fgets(master,50,fp);
        removeNewline(master);
        if(!master[1])
            strcpy(master,crypt("password",SALT));
        fclose(fp);
    }
    return TRUE;

}

char *getExpire( int days )
{

    struct tm	*jst;	/*sys/time.hのtm構造体（時間取得）*/
    time_t		ti;
    static char	expire[BUFSIZE];
    unsigned int year;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT→JST*/

    ti += (86400*days);
    jst = gmtime(&ti);

    if( 0 <= jst->tm_year && jst->tm_year < 70 )		/*2000年問題対策*/
        year = 2000 + jst->tm_year;
    else
        year = 1900 + jst->tm_year;

    sprintf(expire, "%s, %02d-%s-%d %02d:%02d:%02d GMT", eweekday[jst->tm_wday], jst->tm_mday, emonth[jst->tm_mon], year, jst->tm_hour, jst->tm_min, jst->tm_sec);

    return (char *)expire;

}

void setCookieUser( char *handle, char *mail, char *passwd, int confirm )
{

    char *expire;

    expire = getExpire( COOKIE_EXPIRE_DAYS );
    printf("Set-Cookie: %s=name=%s&e-mail=%s&passwd=%s&confirm=%d;" ,ckname, handle, mail, passwd, confirm);
    printf(" Expires=%s; path=/; Domain=%s\r\n" ,expire ,domain);

    return;
}

void setCookieNew( int number )
{

    char *expire;

    expire = getExpire( COOKIE_EXPIRE_DAYS );
    printf("Set-Cookie: n%s=new=%d;" ,ckname ,number);
    printf(" Expires=%s; path=/; Domain=%s\r\n" ,expire ,domain);

    return;

}
/*--- Main ---*/
int main(void)
{

    int		start,end;						/*次のページ表示*/
    int		resnumber = 0;					/*コメント対象の記事番号*/
    int		comnumber = 0;					/*コメント回数*/
    int		chkbox;							/*チェックボックス用のフラグ*/
    char	*title = NULL;		/*記事タイトルへのポインタ*/
    char	writer[HANDLE_MAXSIZE];			/*投稿者*/
    char	pass[PASSWD_LEN];				/*投稿者の削除用パスワードへのポインタ*/
    int		number;							/*記事番号*/
    char	*temp;
    int		i,j,l;							/*カウンタ用*/
    char	fname[BUFSIZE];					/*ファイルネーム作成*/
    FILE	**fp,*fp_read;					/*記事表示・記事書き込み共通*/
    char	*method;						/*REQUEST_METHOD*/
    char	**name, **value;				/*フォームデータ取得用*/
    int		count = -2;						/*フォームデータのname=valueの数*/
    char	*handle = NULL,*address = NULL,*passwd = NULL;		/*クッキーの内容格納*/
    char	*type;							/*タイプ識別*/
    int		ctype = 0;						/*confirmの動作フラグ*/
    int		old_number;						/*前回の既読位置*/
    int		adflag = 0;						/*管理者モードへ入るときのフラグ*/
    char	buffer[BUFSIZE];						/*バッファ*/
    CF		config;

#ifndef DEBUG
    signal( SIGSEGV, fault_abort );
#endif

    DINITL("./file/mewbbslog");
    DPRINTL("--------------------------------------------------------------------------------");

    if(!getConfig(&config))
        return 1;

    setConfig(config);

    if(!getPass())
        return 1;

    method = getenv("REQUEST_METHOD");
    if(method == NULL)
        method = empty;

    number = getCountInt(COUNTER_FILE);	/*現在の記事番号取得*/
    getCookieUser(&chkbox, &handle, &address, &passwd);		/*クッキー取得*/
    old_number = getCookieNew();					/*クッキー取得*/

    /*----- REQUEST_METHODが[POST]の時-----*/
    if(strcmp(method,"POST") == 0) {

        DPRINTL("Start: POST Method.");

        if(!checkReferer())
            return 1;

        count = getForm(&name, &value);		/*stdoutから書き込み内容を取得*/
        if(count == -2 || count == -1) {
            fatal_error("■ フォームデータの取得に失敗しました。", body);
            freedata(name, value);
            return 1;
        }
        if((type = getOption(name)) == NULL) {
            fatal_error("■ 無効なオプションです。", body);
            freedata(name, value);
            return 1;
        }

        if((temp = getValue("confirm", name, value)) != NULL)
            ctype = atoi(temp);
        if((temp = getValue("num", name, value)) != NULL)
            resnumber = atoi(temp);
        if((temp = getValue("adflag", name, value)) != NULL)
            adflag = 1;
        if((temp = getValue("pass", name, value)) != NULL) {
            /*管理者モードに入るときは入力されたパスワードを暗号化し、passwdから暗号化されたパスワードを読み込み認証する。
            通常時は管理者モードに入る時に暗号化されたパスワードをフォームに保存しておき、それをあとから読み出しているので
            そのまま利用し認証する。adflagは管理者モードに入るときに用いるフラグ。*/
            if(adflag == 1)
                strcpy(pass, crypt(temp,SALT));
            else
                strcpy(pass, temp);
        }
        if((temp = getValue("now", name, value)) != NULL)
            now = atoi(temp);
        if((temp = getValue("old", name, value)) != NULL)
            old_number = atoi(temp);

        DPRINTL("before: select type");
        if(!strcmp(type,"write")) {		/*記事登録*/
            if(config.flag_write == 1) { /*書き込みが許可されているとき*/
                fatal_error( "■ 恐れ入りますが、ただいま記事の登録を中止させていただいております。", body);
                return 1;
            }

            if(config.tag == 0 || ctype != 0) {
                for(i = 0; *(value + i); i++) {
                    if(config.tag == 0) {
                        temp = replacestring(*(value + i),"<","&lt;");				/*タグを無効化する*/
                        if( temp != NULL ) {
                            free(*(value + i));
                            *(value + i) = temp;
                        }
                    }
                    if(ctype == 2) {
                        temp = replacestring(*(value + i),(char*)"\"",(char*)"&#34;");	/*"を実体参照(&#34;)へ*/
                        if( temp != NULL ) {
                            free(*(value + i));
                            *(value + i) = temp;
                        }
                    }
                }
            }

            /*--- フォームに入力欄チェック ---*/
            if(**value == '\0') { /* タイトルの入力がない場合 */
                free(*value);
                *value = (char *)malloc( strlen( untitled ) + 1 );
                strcpy( *value, untitled );
            }

            if(!strcmp(strsub(buffer, *(value + 3), 0, 1), " ")) {
                fatal_error("■ ハンドルのはじめに空白は使えません。もう一度、お確かめください。", body);
                freedata(name, value);
                return 1;
            }
            if(!strcmp(strsub(buffer, *(value + 3), 0, 2), "　")) {
                fatal_error("■ ハンドルのはじめに空白は使えません。もう一度、お確かめください。", body);
                freedata(name, value);
                return 1;
            }

            if(**(value + 3) == '\0') {	/* ハンドルの入力がない場合 */
                fatal_error("■ ハンドルの入力は必須です。もう一度、お確かめください。", body);
                freedata(name, value);
                return 1;
            }
            if(check_url(*(value + 2)) == 0) {
                fatal_error("■ URLの形式形式が違います。もう一度、お確かめください。", body);
                exit(1);
            }

            temp = replacestring(*value ,"<","&lt;");				/*タイトルのタグを無効化する*/
            if( temp != NULL ) {
                free(*value);
                *value = temp;
            }
            /*--- 登録確認フォーム出力 ---*/
            if(ctype == 2) {
                formConfirm(name, value, count);
                freedata(name, value);
                return 0;
            } else if(ctype == 1) {
                chkbox = 1;
            } else {
                chkbox = 0;
            }

            setCookieUser( *(value + 3), *(value + 4), passwd, chkbox );

            if(!writeDescription( number + 1, value, config)) {
                freedata(name, value);
                return 1;
            }

            if(putCountInt(COUNTER_FILE, number++)) {
                freedata(name, value);
                fatal_error("■ カウンターファイルに書き込み権限が与えられていません。", body);
                return 1;
            }
            limitDescription(number, config.regmax);


            free(handle);
            free(address);
            handle = *(value + 3);
            address = *(value + 4);

        } else if(!strcmp(type,"reload")) {			/*更新*/
            ;
        } else if(!strcmp(type,"home")) {				/*掲示板終了*/
            printf("Location: %s\n\n" ,config.home_url);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"del")) {				/*記事削除*/
            if(config.flag_write == 0) {
                if(!deleteDescription(count, name )) {
                    freedata(name, value);
                    return 1;
                }
            }
            if(!formDelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fdel")) {			/*記事削除用フォーム*/
            if(!formDelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fchcfg")) {			/*設定変更用フォーム*/
            formConfig(config);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"chcfg")) {			/*設定変更*/
            if(!chConfig(value)) {
                freedata(name, value);
                return 1;
            }
            if(!getConfig(&config)) {
                freedata(name, value);
                return 1;
            }
            if(!formAdmin(pass)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"chsec")) {			/*PROXY経由投稿禁止設定変更*/
            chSecurity(name, value, count);
            if(!getConfig(&config)) {
                freedata(name, value);
                return 1;
            }
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fundel")) {		/*記事復活用フォーム*/
            if(!formUndelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fadmin")) {			/*管理者モードフォーム*/
            if(!getConfig(&config))
                return 1;
            if(!formAdmin(pass)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"undel")) {			/*記事復活*/
            if(config.flag_write == 0) {
                if(!undelDescription(count, name)) {
                    freedata(name, value);
                    return 1;
                }
            }
            if(!formUndelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fcorselect")) {			/*記事修正用一覧*/
            if(!formCorSelect(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fcorinput")) {			/*記事修正用フォーム*/
            if(!formCorInput(resnumber)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"correction")) {		/*記事修正*/
            if(config.flag_write == 0) {
                if(!correctDescription(value)) {
                    freedata(name, value);
                    return 1;
                }
            }
            if(!formCorSelect(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fchpass")) {			/*パスワード変更用フォーム*/
            formChpass();
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"chpass")) {			/*パスワード変更*/
            if(!chPass(value)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "fsecurity")) {			/*投稿禁止ホスト追加・削除フォーム*/
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "delhosts")) {		/*投稿禁止ホスト削除*/
            if(!deleteHosts( name, value )) {
                freedata(name, value);
                return 1;
            }
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "addhost")) {			/*投稿禁止ホスト追加*/
            DPRINTL("before: addHost()");
            if(!addHost( name, value )) {
                DPRINTL("addHost failed.");
                freedata(name, value);
                DPRINTL("End: freedata()");
                return 1;
            }
            formSecurity(config.proxy, config.hostchk);
            DPRINTL("End: formSecurity()");
            freedata(name, value);
            DPRINTL("End: freedata()");
            return 0;
        } else if(!strcmp(type,"res")) {				/*コメント*/
            if(resnumber > 0) {
                sprintf(fname,"./file/%d",resnumber);

                if((fp_read = fopen(fname,"r")) == NULL) {
                    fatal_error("■ コメント先の番号が指定されていないか、該当する記事はありません。", body);
                    return 1;
                }
                for( i = 0; i < 6; i++ ) {
                    if(fgets(buffer,BUFSIZE,fp_read) == NULL) {
                        if(ferror(fp_read)) {
                            fatal_error("■ 何らかのエラーが発生しました。<BR>同じようなエラーが何度も出る場合は管理者に連絡してください。(error code 340)", body);
                            return 1;
                        }
                        if(feof(fp_read))
                            break;
                    }
                    removeNewline(buffer);

                    if(i == 2)
                        strcpy(writer,buffer);
                    else if(i == 4)
                        comnumber = atoi(buffer) + 1;
                    else if(i == 5) {
                        if(!(title = createTitle(buffer)))
                            fatal_error( "■ システムエラーです。再度実行してください。", body );
                    }
                }
                fclose(fp_read);
            }
        }
    } else {
        type = "Unknown";
    }
    /*表示する記事の数設定（現在の記事数-maxが１以下だった場合は０に設定）*/

    DPRINTL("Start: GET Method.");

    if(!strcmp(type,"next")) {	/*次のページを表示*/
        start = now;
        end = start - config.max;
        if(end < 1)
            end = 0;
    } else {	/*通常*/
        start = number;
        if(start < config.max)
            end = 0;
        else
            end = start - config.max;
    }

    setCookieNew( number );
    DPRINTL("End: Send Cookie.");

    printPageHeader(rmHtmlTag(config.aptitle));
    puts(body);
    printf("<BASEFONT SIZE=%d>\n", config.fsize);
    printf(
        "<BR>\n"
        "<FONT SIZE=\"+3\">\n"
        "<STRONG>%s</STRONG>\n"
        "</FONT>\n"
        "<BR>\n"
        ,config.maintitle);
    printf("%s\n<HR>\n" ,config.subtitle);

    printf(
        "\n<!-- 投稿フォーム -->\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">"
        "<TABLE BORDER=3 CELLPADDING=3 CELLSPACING=3>\n"
    );

    if(resnumber != 0) {
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>題名</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"title\" VALUE=\"%s\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            "</TR>\n"
            ,title, TITLE_SIZE, TITLE_MAXSIZE
        );
        printf(
            "<TR>\n"
            "	<TD VALIGN=\"TOP\" NOWRAP>\n"
            "		<STRONG>内容</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<TEXTAREA NAME=\"comment\" ROWS=%d COLS=%d>"
            , CONTENTS_ROWS, CONTENTS_COLS
        );

        if(!quoteDescription(resnumber)) {
            freedata(name, value);
            return 1;
        }

        printf(
            "</TEXTAREA>\n"
            "	</TD>\n"
            "</TR>\n"
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>参照アドレス</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"url\" VALUE=\"http://\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            "</TR>\n"
            , URL_SIZE, URL_MAXSIZE
        );
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>名前</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"handle\" VALUE=\"%s\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            ,handle, HANDLE_SIZE, HANDLE_MAXSIZE
        );
        printf(
            "</TR>\n"
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>e-mail</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"mail\" VALUE=\"%s\" SIZE=%d MAXLENGTH=%d>\n"
            , address, MAIL_SIZE, MAIL_MAXSIZE
        );
        printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"passwd\" VALUE=\"%s\">\n", passwd);
        printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"resnum\" VALUE=\"%05d\">\n",resnumber);
        printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"writer\" VALUE=\"%s\">\n",writer);
        printf("		<INPUT TYPE=\"HIDDEN\" NAME=\"comnum\" VALUE=\"%d\">\n",comnumber);
        printf(
            "	</TD>\n"
            "</TR>\n"
        );

        if(chkbox == 1) {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"checkbox\" NAME=\"confirm\" VALUE=\"2\" CHECKED>\n"
                "		<STRONG>登録内容を確認</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        } else {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"checkbox\" NAME=\"confirm\" VALUE=\"2\">\n"
                "		<STRONG>登録内容を確認</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        }
        printf(
            "<TR>\n"
            "	<TD COLSPAN=2>\n"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"記事#%dへ返信\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"reload\" VALUE=\"消去 ／ 更新\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">"
            "	</TD>\n"
            "</TR>\n"
            ,resnumber
        );
    } else {
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>題名</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"title\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            "</TR>\n"
            , TITLE_SIZE, TITLE_MAXSIZE
        );
        printf(
            "<TR>\n"
            "	<TD VALIGN=\"TOP\" NOWRAP>\n"
            "		<STRONG>内容</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<TEXTAREA NAME=\"comment\" ROWS=%d COLS=%d></TEXTAREA>\n"
            "	</TD>\n"
            "</TR>\n"
            , CONTENTS_ROWS, CONTENTS_COLS
        );
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>参照アドレス</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"url\" VALUE=\"http://\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            "</TR>\n"
            ,URL_SIZE, URL_MAXSIZE
        );
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>名前</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"handle\" VALUE=\"%s\" SIZE=%d MAXLENGTH=%d>\n"
            "	</TD>\n"
            "</TR>\n"
            ,handle ,HANDLE_SIZE, HANDLE_MAXSIZE
        );
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>e-mail</STRONG>\n"
            "	</TD>\n"
            "	<TD>\n"
            "		<INPUT TYPE=\"TEXT\" NAME=\"mail\" VALUE=\"%s\" SIZE=%d MAXLENGTH=%d>\n"
            , address, MAIL_SIZE, MAIL_MAXSIZE
        );
        printf(
            "		<INPUT TYPE=\"HIDDEN\" NAME=\"passwd\" VALUE=\"%s\">\n"
            "		<INPUT TYPE=\"HIDDEN\" NAME=\"resnum\" VALUE=\"･････\">\n"
            "		<INPUT TYPE=\"HIDDEN\" NAME=\"writer\" VALUE=\"-----\">\n"
            "		<INPUT TYPE=\"HIDDEN\" NAME=\"comnum\" VALUE=\"0\">\n"
            "	</TD>\n"
            "</TR>\n"
            ,passwd
        );
        if(chkbox == 1) {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"CHECKBOX\" NAME=\"confirm\" VALUE=\"2\" CHECKED>\n"
                "		<STRONG>登録内容を確認</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        } else {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"CHECKBOX\" NAME=\"confirm\" VALUE=\"2\">\n"
                "		<STRONG>登録内容を確認</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        }
        printf(
            "<TR>\n"
            "	<TD COLSPAN=2>\n"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"投　稿  す  る\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"reload\" VALUE=\"消去 ／ 更新\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"ホームページへ\">\n"
            "	</TD>\n"
            "</TR>\n"
        );
    }
    printf("</TABLE>\n</FORM>\n");
    printf("\n<!-- ステータス表\示 -->\n");
    printStatus(start, end, number, old_number, type, value, config);	/*ステータス表示*/
    printf("<HR SIZE=3>\n\n");

    /*--- 記事を表示 ---*/
    fp = (FILE **)malloc(sizeof(FILE *) * config.max);
    for(j = start,l = 0 ; j > end; j--, l++) {	/*ファイルポインタをまとめてオープン*/
        sprintf(fname,"./file/%d",j);
        if((*(fp + l) = fopen(fname,"r")) == NULL)
            continue;
    }
    for(j = start, l = 0 ; j > end; j--, l++) {
        if(*(fp + l) == NULL)
            continue;
        if(!printDescription(j, *(fp + l), old_number, config))
            continue;
    }
    for(j = start,l = 0 ; j > end; j--, l++) {	/*ファイルポインタをまとめてクローズ*/
        if(*(fp + l) == NULL)
            continue;
        fclose(*(fp + l));
    }
    free(fp);
    /*--- 記事表\示終了---*/

    printf("\n<!-- コントロールフォーム表\示 -->\n");
    printForm2(start, end, config.regmax, config.max, old_number, number);	/*記事表\示終了後のフォーム出力*/
    puts("\n<HR>");

    /*----- 著作権表示（削除しないでください）----- */
    puts(
        "\n<!-- 著作権表\示 -->\n"
        "<P ALIGN=RIGHT><A HREF=\"http://www.onbiz.net/~mew/\">\n<STRONG>mewBBS Ver4.94a</STRONG>\n</A></P>"
    );
    /*----------------------------------------------*/
    puts("\n</BODY>\n</HTML>");

    if(count >= 0) {
        freedata(name, value);
    } else {
        free(address);
        free(handle);
        free(passwd);
    }

    return 0;	/*--- 終了 ---*/
}
/*End Of File*/
