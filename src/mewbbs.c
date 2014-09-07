/*---------------------------------------------------------------------------
 * mewbbs.c Ver4.94a
 * ���@�\�f����
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

static const char	*weekday[]= {"��","��","��","��","��","��","�y"}; /* �\������j���̕��� */
static const char	*eweekday[]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"}; /* �\������j���̕��� */
static const char	*emonth[]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char	*atype[]= {"write","reload","home","next","res","del","fadmin","fchcfg","chcfg","chsec","fundel","fdel","undel","correction","fcorinput","fcorselect","fchpass","chpass","fsecurity", "delhosts", "addhost",NULL,};
char				master[PRF_SIZE];			/*�Ǘ��҃p�X���[�h*/
char				scurl[PRF_SIZE];			/*�X�N���v�g��URL*/
char				domain[PRF_SIZE];			/*Cookie���s�p��DOMAIN*/
char				ckname[PRF_SIZE];			/*Cookie���s�p��NAME*/
int					now;						/*���ݕ\�����̍ŐV�L���ԍ�*/

char	body[PRF_SIZE];		/*�\���F(HTML��BODY�^�O)*/
char	font[PRF_SIZE];


/*-----------------------------------------------------------------------------
�v���g�^�C�v�Fvoid fault_abort( int sig );
����		�Fint sig		���������V�O�i��
����		�F���������V�O�i�����󂯁Aexit����B�i���̃X�N���v�g�ł�SIGSEGV�j
-----------------------------------------------------------------------------*/
void fault_abort( int sig )
{

    fatal_error(
        "�X�N���v�g�ɒv���I�ȃG���[���������܂����B�ēx���s���Ă��������B<BR>\n"
        "�Ď��s����������b�Z�[�W���\\�������ł���΁A�������܂����Ǘ��҂܂ł��A�����������B<BR>\n"
        "<BR>\n"
        "Myu's Lab.: http://www.onbiz.net/~mew/<BR>\n", body);
    exit(1);

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid addHost (int, char **, char **)
�����@�@�@�@�Fint count			�t�H�[���ő��M���ꂽname=value�̐�
�@�@�@�@�@�@�@char **name 		�t�H�[���ő��M���ꂽname
�@�@�@�@�@�@�@char **value 		�t�H�[���ő��M���ꂽvalue
�����@�@�@�@�F���e�֎~�z�X�g��ǉ�����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int addHost( char **name, char **value )
{

    int		i;

    DPRINTL("Start: addHost()");
    for(i = 0; *(name + i); i++) {
        if(!strcmp(*(name + i),"host")) {
            if(strlen(*(value + i)) < 1) {
                fatal_error("�� �ǉ�����z�X�g�����͂���Ă��܂���B", body);
                return FALSE;
            }
            addLine(DENIED_LIST, *(value + i));
        }
    }
    DPRINTL("End: addHost()");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid deleteHosts(int, char **)
�����@�@�@�@�Fchar **name		�t�H�[���ő����Ă����폜�ΏۋL���ԍ�
�����@�@�@�@�F���e�֎~�z�X�g�̍폜
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int deleteHosts( char **name, char **value )
{

    int		i;				/*�J�E���^*/
    int		delnumber;		/*�폜�Ώۍs*/
    int		delcount = 0;	/*���s��������*/
    int		flg_cnt = 0;

    for(i = 0; *(name + i); i++) {
        if( **(name + i) == '_' && *(*(name + i) + 1) == '_' )
            flg_cnt++;
    }
    if( flg_cnt == 0 ) {
        fatal_error("�� �폜���铊�e�֎~�z�X�g��I�����Ă��������B", body);
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
�v���g�^�C�v�Fchar *getOption(char **, int)
�����@�@�@�@�Fchar **name		�t�H�[���̓��e����擾���ꂽ����I�v�V����
�@�@�@�@�@�@�@int count			�t�H�[���̓��e�̌�
�����@�@�@�@�F�t�H�[�����瑗�M���ꂽ���e���瓮��I�v�V�������擾���A�Ԃ��B
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
�v���g�^�C�v�Fvoid setConfig (CF)
�����@�@�@�@�FScript��URL(scurl)�ACookie���s����NAME(ckname)�ADOMAIN(domain)��ݒ�
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
�v���g�^�C�v�Fvoid getConfig(CF *)
�����@�@�@�@�Fmewbbs.cf����ݒ��ǂݍ��ށB
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getConfig(CF *config)
{

    char	**file2Dim;

    file2Dim = readFile( CONFIG_FILE );
    if( file2Dim == NULL ) {
        fatal_error("�ݒ�t�@�C�������݂��Ȃ����Aother�ɏ������݌������^�����Ă��܂���B","<BODY>");
        return FALSE;
    }

    if(getCfValueStr( file2Dim, "APTITL", config -> aptitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> aptitle, "�^�C�g�����ݒ�");
    if(getCfValueStr( file2Dim, "MAINTITL", config -> maintitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> maintitle, "�^�C�g�����ݒ�");
    if(getCfValueStr( file2Dim, "SUBTITL", config -> subtitle, MAX_CONFIG_LEN) == NULL)
        strcpy(config -> subtitle, "�T�u�^�C�g�����ݒ�");
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
�v���g�^�C�v�Fvoid chConfig(char *)
�����@�@�@�@�Fchar **value		�t�H�[�����瑗�M���ꂽ�ݒ���e
�����@�@�@�@�Fcf�̏�������������B
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
        fatal_error("�� ��x�ɕ\\������L������1�`50�̊Ԃɐݒ肵�Ă��������B" ,body);
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
�v���g�^�C�v�Fvoid chSecurity(char **)
�����@�@�@�@�Fchar **value		�t�H�[�����瑗�M���ꂽ�ݒ���e
�����@�@�@�@�Fcf�̏�������������B
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
�v���g�^�C�v�Fvoid getCookies(int *, char **, char **, char **)
�����@�@�@�@�Fint *ctype		�m�F�t�H�[���̃^�C�v�ւ̃|�C���^
�@�@�@�@�@�@�@char **handle		���e�҃n���h���փ|�C���^
�@�@�@�@�@�@�@char **address	���e�҃��[���A�h���X�ւ̃|�C���^
�@�@�@�@�@�@�@char **passwd		���e�҂����͂����p�X���[�h�ւ̃|�C���^
�����@�@�@�@�FCookie�����L�̈������̒l�𒲂ׁA�Z�b�g����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*--handle, address, passwd�͌Ăяo�����֐��ɂė̈������K�v--*/
void getCookieUser(int *ctype, char **handle, char **address, char **passwd)
{

    int		count;				/*Cookie�𕪊�����name*value�̑g��*/
    char	**cname = NULL, **cvalue = NULL;	/*Cookie�𕪊������l*/
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
�v���g�^�C�v�Fint getCookieNew(int)
�Ԓl�@�@�@�@�Fint...���ǈʒu
�����@�@�@�@�F���ǈʒu���N�b�L�[����ǂݍ��݁A�Ԃ�
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCookieNew(void)
{

    char	*temp;
    int		old_number = 0;
    int		count;				/*Cookie�𕪊�����name*value�̑g��*/
    char	**cname = NULL, **cvalue = NULL;	/*Cookie�𕪊������l*/
    char	c_name[BUFSIZE];	/*���ǃ|�C���^�ۑ��pCookie�̖��O*/

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
�v���g�^�C�v�Fvoid checkReferer(void)
�����@�@�@�@�FSCURL�Ə������݂���Ă���URL���r���A�قȂ�URL����̏ꍇ�͓��e
�@�@�@�@�@�@�@���֎~����B�i�O���t�H�[������̓o�^�֎~�j
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int checkReferer(void)
{

    char	*referer;		/*HTTP_REFERER*/
    char	*buffer;
    char	*buf_ref;

    referer = getenv("HTTP_REFERER");

    if(referer == NULL || *referer == '\0') {
        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 100)", body);
        return FALSE;
    } else {
        buffer = (char *)malloc(strlen(referer) + 1);
        buf_ref = (char *)malloc(strlen(referer) + 1);
        strcpy(buf_ref, referer);
        strspl(buffer,buf_ref,'?');
        decode(buffer);

        if(strcasecmp(buffer,scurl) != 0) {
            printPageHeader("�O���t�H�[������̏������݋֎~");
            puts(body);
            printf("<BR>���Ȃ����������݂��ꂽ�A�h���X�́A<STRONG>%s</STRONG>�ł��B",buffer);
            printf("<BR>�\\���󂠂�܂���<A HREF=\"%s\"><STRONG>%s</STRONG></A>���珑�����݂����肢���܂��B",scurl,scurl);
            return FALSE;
        }
        free(buffer);
        free(buf_ref);
    }

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar createTitle(char *)
�����@�@�@�@�Fchar *com		���L���̃^�C�g��
			�Fchar *title	�쐬���ꂽ�^�C�g���i����com+1���m�ۂ���Ă���K�v������j
�߂�l�@�@�@�F����	�쐬���ꂽ�^�C�g��
�@�@�@�@�@�@�@���s	NULL
�����@�@�@�@�F���X�|���X���ꂽ�񐔂��܂߂��^�C�g���̍쐬
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *createTitle( char *com )
{

    char	buf[BUFSIZE];		/*�쐬�����^�C�g��*/
    char	*buf_title;
    char	re1[] = "RE:";
    char	re2[] = "RE";
    int		iresnum;		/*�R�����g��*/
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
�v���g�^�C�v�Fvoid printStatus(int, int, int, int, char *, char **)
�����@�@�@�@�Fint start			�\������n�߂̋L���ԍ�
�@�@�@�@�@�@�@int end			�\������Ō�̋L���ԍ�
�@�@�@�@�@�@�@int number		�ŐV�̋L���ԍ�
�@�@�@�@�@�@�@int old_number	�O��\�������ŏI�ԍ��i���ǈʒu�j
�@�@�@�@�@�@�@char *type		����I�v�V����
�@�@�@�@�@�@�@char **value		�t�H�[�����瑗�M���ꂽ���e
�����@�@�@�@�F�f���̃X�e�[�^�X�o��
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void printStatus(int start, int end, int number, int old_number, char *type, char **value, CF config)
{

    int		unread;

    if(old_number != 0) {
        unread = (number - old_number)-(number - start);
        if(unread <= 0)
            unread = 0;
        if(unread <= 0) {
            printf("�� ���ǂ͂���܂���B�i���F���ǋL���@���F���ǋL���j<BR>\n");
        } else {
            printf("�� ���ǂ�<STRONG>%d��</STRONG>����܂��B�i���F���ǋL���@���F���ǋL���j<BR>\n",unread);
        }
    }
    if(config.flag_write == 1)
        puts("�� ���݁A���e���֎~���Ă��܂��B<BR>\n");
    if(!strcmp(type,"write"))
        printf("�� <STRONG>%s����</STRONG>����̓��e�w<STRONG>%s</STRONG>�x��<STRONG>%d��</STRONG>�ɓo�^���܂����B<BR>\n",*(value + 3),*value,number);
    if(!strcmp(type,"del"))
        printf("�� �L���ԍ�<STRONG>%d</STRONG>���폜���܂����B<BR>\n",atoi(*(value + 1)));
    if(config.tag == 0)
        puts("�� HTML�^�O�̎g�p���֎~���Ă��܂��B<BR>\n");
    if(config.regmax != 0)
        printf("�� �ő�o�^������<STRONG>%d��</STRONG>�ł��B������z����ƍł��Â��L�����玩���I�ɍ폜����܂��B<BR>\n",config.regmax);
    if(start > (end + 1))
        printf("�� ���݁A<STRONG>%d</STRONG>�`<STRONG>%d</STRONG>�܂ł�<STRONG>%d��</STRONG>��\\�����Ă��܂��B�i�폜�L���͕\\�����܂���j\n",end+1,start,start-end);
    else if(start == (end + 1))
        printf("�� ���݁A�L���ԍ�<STRONG>%d</STRONG>��\\�����Ă��܂��B\n",start);
    puts("</FONT>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid printForm2(int, int, int)
�����@�@�@�@�Fint number		�ŐV�̋L���ԍ�
�@�@�@�@�@�@�@int end			�o�͂���Ō�̋L���ԍ�
�@�@�@�@�@�@�@int old_number	�O��\�������ŏI�ԍ��i���ǈʒu�j
�����@�@�@�@�F�L���o�͏I����̃t�H�[���o��
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
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">\n"
                "	</TD>\n"
            );
        } else {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"����%d����\\��\">\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"now\" VALUE=\"%d\">\n"
                "		<INPUT TYPE=\"HIDDEN\" NAME=\"old\" VALUE=\"%d\">\n"
                "	</TD>\n"
                ,max ,end ,old_number);
        }
    } else {
        if( (start - max) <= (number - regmax) ) {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">\n"
                "	</TD>\n"
            );
        } else {
            printf(
                "	<TD NOWRAP>\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"����%d����\\��\">\n"
                "		<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">\n"
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
        "		<INPUT TYPE=\"SUBMIT\" VALUE=\"�Ǘ��҃��[�h\">\n"
        "	</TD>\n"
        "	<TD WIDTH=5>\n"
        "		�@\n"
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
�v���g�^�C�v�Fvoid writeDescription(int, struct tm, char **)
�����@�@�@�@�Fint number		�L���ԍ�
�@�@�@�@�@�@�Fstruct tm *gmt	tm�\���̂ւ̃|�C���^
�@�@�@�@�@�@�Fchar **value		�t�H�[������̑��M���e
�����@�@�@�@�F�L����o�^����
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int writeDescription(int number , char **value, CF config)
{

    char	fname[BUFSIZE];					/*�t�@�C�����쐬�p*/
    char	*remoteaddr;
    char	*remotehost;
    char	*useragent;
    FILE	*fp;						/*�L����������*/
    int		i;
    char	buffer[BUFSIZE];
    unsigned int	flag = 0;
    struct tm	*jst;	/*sys/time.h��tm�\���́i���Ԏ擾�j*/
    time_t		ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT��JST*/
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

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=�v���N�V�o�R�̓��e�֎~����-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
    if(config.proxy == 1) {
        if (!ViaProxy ()) {
            fatal_error("�� PROXY�T�[�o����̏������݂͋�����Ă��܂���B",body);
            return FALSE;
        }
        if (!IsProxy ()) {
            fatal_error("�� PROXY�T�[�o����̏������݂͋�����Ă��܂���B",body);
            return FALSE;
        }
    } else if(config.proxy == 2) {
        if(!isAnonymousProxy()) {
            fatal_error("�� ����PROXY�T�[�o����̏������݂͋�����Ă��܂���B",body);
            return FALSE;
        }
    }
    /*-=-=-=-=-=-=-=-=-=-=-=-=�w�肵���z�X�g����̓��e�֎~����-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if((fp = fopen(DENIED_LIST, "r")) == NULL) {
        fatal_error("�� ���e�֎~�z�X�g���X�g(denied)�����݂��Ȃ����A�ǂݍ��݌������^�����Ă��܂���B",body);
        return FALSE;
    }

    for(i = 0;  ; i++) {
        if(fgets(buffer,BUFSIZE,fp) == NULL)
            break;
        removeNewline(buffer);
        if(strstr(remoteaddr, buffer) != NULL ) {
            fatal_error("�� ���̃z�X�g����̓��e�͋֎~����Ă��܂��B",body);
            return FALSE;
        } else if(strstr(remotehost, buffer) != NULL ) {
            fatal_error("�� ���̃z�X�g����̓��e�͋֎~����Ă��܂��B",body);
            return FALSE;
        }
    }
    fclose(fp);

    /*-=-=-=-=-=-=-=-=-=-=-=-=�z�X�g������t�������ċU�z�X�g����̓��e�֎~����-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if( config.hostchk == 1 ) {
        if (!ReverseDNS()) {
            fatal_error("�� �s���ȃz�X�g����̓��e�͋֎~����Ă��܂��B", "<BODY>");
            return FALSE;
        }
    }

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=�A�����e�֎~����-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    if((number - 1) > 1) {
        sprintf(fname,"./file/%d",number - 1);
        if((fp = fopen(fname,"r")) != NULL) {

            for(i = 0; i < 6; i++) {
                if(fgets(buffer,BUFSIZE,fp) == NULL) {
                    if(ferror(fp)) {
                        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 152)",body);
                        return FALSE;
                    }
                    if(feof(fp))
                        break;
                }
                removeNewline(buffer);
                if( i == 2 ) {
                    if(strcmp( buffer, *(value + 3) ) != 0)		/*���e�Ҕ�r*/
                        flag = 1;
                } else if( i == 3 ) {
                    if(strcmp( buffer, *(value + 4) ) != 0)		/*���[���A�h���X��r*/
                        flag = 1;
                } else if( i == 5 ) {
                    if(strcmp( buffer, *value ) != 0)			/*�^�C�g����r*/
                        flag = 1;
                } else if( i == 6 ) {								/*REMOTE_ADDR��r*/
                    if(strcmp( buffer, remoteaddr ) != 0)
                        flag = 1;
                }
            }
            fclose(fp);
            if( flag == 0 ) {
                fatal_error("�� �������e�҂��瓯�����e��A�����ē��e�ł��܂���B",body);
                return FALSE;
            }
        }
    }

    /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

    sprintf(fname,"./file/%d",number);	/*path���܂߂��t�@�C�����쐬*/
    sprintf(buffer, "./file/%dcp", number);

    if(access(fname, 00) == 0)
        rename(fname, buffer);

    if((fp = fopen(fname,"w")) == NULL) {
        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 153)",body);
        return FALSE;
    }

    /*�L�����e���t�H�[�}�b�g�ɏ]���ď�������*/
    fprintf(fp,"%05d\n",number);		/*[0]�L���ԍ�*/
    fprintf(fp,"%02d��%02d��(%s)%02d��%02d��\n",jst->tm_mon+1,jst->tm_mday,weekday[jst->tm_wday],jst->tm_hour,jst->tm_min);/*[1]���e����*/
    fprintf(fp,"%s\n",*(value + 3));	/*[2]���e��*/
    fprintf(fp,"%s\n",*(value + 4));	/*[3]���[���A�h���X*/
    fprintf(fp,"%s\n",*(value + 8));	/*[4]���g�p*/
    fprintf(fp,"%s\n",*value);			/*[5]�^�C�g��*/
    fprintf(fp,"%s\n",useragent);		/*[6]���[�U�[�G�[�W�F���g*/
    fprintf(fp,"%s\n",remoteaddr);		/*[7]�����[�g�z�X�g*/
    fprintf(fp,"%s\n",*(value + 6));	/*[8]�R�����g���ԍ�*/
    fprintf(fp,"%s\n",*(value + 7));	/*[9]�R�����g�����e��*/
    fprintf(fp,"0\n");					/*[10]�L�������i�����͒ʏ�L���Ȃ̂�0�j*/
    fprintf(fp,"%s\n",*(value + 2));	/*[11]URL*/
    fprintf(fp,"\n\n\n");				/*[12-14]�c��̖��g�p�̈�̏o�́i���s�̂݁j*/
    fprintf(fp,"%s",*(value + 1));		/*[15]�R�����g*/
    fclose(fp);

    return TRUE;
}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid limitDescription(int)
�����@�@�@�@�Fint number	�ŐV�̋L���ԍ�
�����@�@�@�@�F�o�^�������z�����L���̍폜
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*--- �o�^���������z�����L���̍폜 ---*/
void limitDescription(int number, int regmax)
{

    char	fname[BUFSIZE];
    int		i;			/*�J�E���^*/

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
�v���g�^�C�v�Fvoid deleteDescription(int, char **, struct tm)
�����@�@�@�@�Fint count			�폜�̑ΏۂƂȂ�L����
�@�@�@�@�@�@�@char **name		�t�H�[���ő����Ă����폜�ΏۋL���ԍ�
�@�@�@�@�@�@�@struct tm *jst	tm�\���̂ւ̃|�C���^
�����@�@�@�@�F�L�����폜���A�o�b�N�A�b�v�t�@�C�����쐬����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int deleteDescription(int count, char **name )
{

    int		i,j;				/*�J�E���^*/
    int		delnumber;			/*�폜�ΏۋL���ԍ�*/
    char	buffer[BUFSIZE];	/*�t�@�C���ǂݍ��݃o�b�t�@*/
    char	fname[BUFSIZE];		/*�t�@�C�����쐬�p*/
    FILE	*fp;				/*�폜�L����ǂݍ���*/
    FILE	*fpbackup;			/*�o�b�N�A�b�v�t�@�C���ɏ�������*/
    struct tm	*jst;	/*sys/time.h��tm�\���́i���Ԏ擾�j*/
    time_t		ti;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT��JST*/
    jst = gmtime(&ti);

    if(count < 2) {
        fatal_error("�� �폜����L����I�����Ă��������B", body);
        return FALSE;
    }

    for(i = 0; i < (count - 1) ; i++) {
        delnumber = atoi(*(name + i));
        if(delnumber == 0)
            continue;
        sprintf(fname, "./file/%d", delnumber);		/*path���܂߂��폜�Ώۃt�@�C�����쐬*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("�� �폜��̔ԍ����w�肳��Ă��Ȃ����A�Y������L���͂���܂���B", body);
            return FALSE;
        } else {
            for(j = 0;  ; j++) {
                if(fgets(buffer, BUFSIZE, fp) == NULL) {
                    if(ferror(fp)) {
                        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 170)", body);
                        return FALSE;
                    }
                    if(feof(fp))
                        break;
                }
                if(j == 10)
                    break;
            }
            if(atoi(buffer) == 1 || atoi(buffer) == 2) {	/*�폜�L��*/
                fclose(fp);
                continue;
            }
        }
        fclose(fp);

        sprintf(fname, "./file/%d", delnumber);		/*path���܂߂��폜�Ώۃt�@�C�����쐬*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("�� �폜��̔ԍ����w�肳��Ă��Ȃ����A�Y������L���͂���܂���B", body);
            return FALSE;
        }
        sprintf(fname, "./file/%ddel", delnumber);	/*path���܂߂��o�b�N�A�b�v�t�@�C�����쐬*/
        if((fpbackup = fopen(fname, "w")) == NULL) {
            fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 171)", body);
            return FALSE;
        }
        /*--- �폜�Ώۃt�@�C����ǂݍ��݁A�o�b�N�A�b�v�t�@�C���ɏ������� ---*/
        for(j = 0;  ; j++) {
            if(fgets(buffer, BUFSIZE, fp) == NULL) {
                if(ferror(fp)) {
                    fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 172)", body);
                    return FALSE;
                }
                if(feof(fp))
                    break;
            }
            fputs(buffer,fpbackup);
        }
        fclose(fp);
        fclose(fpbackup);

        /*--- �폜�L���̃t�H�[�}�b�g�ʂ�ɏo�� ---*/
        sprintf(fname,"./file/%d",delnumber);
        if((fp = fopen(fname,"w")) == NULL) {
            fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 173)", body);
            return FALSE;
        }
        fprintf(fp,"%05d\n",delnumber);
        fprintf(fp,"%02d��%02d��(%s)%02d��%02d��\n",jst->tm_mon+1,jst->tm_mday,weekday[jst->tm_wday],jst->tm_hour,jst->tm_min);
        fputs("�Ǘ��ҍ폜\n",fp);
        fputs("\n",fp);/*���[���A�h���X*/
        fputs("\n",fp);/*�R�����g��*/
        fputs("�Ǘ��ҍ폜\n",fp);
        fputs("\n",fp);/*HTTP_USER_AGENT*/
        fputs("\n",fp);/*REMOTE_HOST*/
        fputs("�����\n",fp);/*�R�����g���ԍ�*/
        fputs("-----\n",fp);/*�R�����g�����e��*/
        fputs("2\n",fp);/*�Ǘ��ҍ폜*/
        fputs("\n\n\n\n\n",fp);
        fclose(fp);
    }
    return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid undelDescription(int, char **)
�����@�@�@�@�Fint count		�폜�̑ΏۂƂȂ�L����
�@�@�@�@�@�@�Fchar **name	�t�H�[���ő����Ă����폜�ΏۋL���ԍ�
�����@�@�@�@�F�폜�L���𕜊����A�o�b�N�A�b�v�t�@�C�����폜����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int undelDescription(int count, char **name)
{

    int		i,j;				/*�J�E���^*/
    int		undelnumber;		/*�����Ώۍ폜�L��*/
    char	buffer[BUFSIZE];	/*�t�@�C���ǂݍ��݃o�b�t�@*/
    char	fname[BUFSIZE];			/*�t�@�C�����쐬�p*/
    FILE	*fp;				/*�폜�L����ǂݍ���*/
    FILE	*fpbackup;			/*�o�b�N�A�b�v�t�@�C���ɏ�������*/

    if(count < 2) {
        fatal_error("�� ����������폜�L����I�����Ă��������B", body);
        return FALSE;
    }

    for(i = 0; i < (count - 1) ; i++) {
        undelnumber = atoi(*(name + i));
        if(undelnumber == 0)
            continue;
        sprintf(fname, "./file/%d", undelnumber);	/*path���܂߂��o�b�N�A�b�v�t�@�C�����쐬*/
        if((fpbackup = fopen(fname, "w")) == NULL) {
            fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 180)", body);
            return FALSE;
        }
        sprintf(fname, "./file/%ddel", undelnumber);		/*path���܂߂��폜�Ώۃt�@�C�����쐬*/
        if((fp = fopen(fname,"r")) == NULL) {
            fatal_error("�� �����L���̔ԍ����w�肳��Ă��Ȃ����A�Y������L���͂���܂���B", body);
            return FALSE;
        }
        /*--- �폜�Ώۃt�@�C����ǂݍ��݁A�o�b�N�A�b�v�t�@�C���ɏ������� ---*/
        for(j = 0;  ; j++) {
            if(fgets(buffer, BUFSIZE, fp) == NULL) {
                if(ferror(fp)) {
                    fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 181)", body);
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
�v���g�^�C�v�Fvoid quoteDescription(int)
�����@�@�@�@�Fint resnumber		���p����Ώۂ̋L���ԍ�
�����@�@�@�@�F�Y������L�������p���A�o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int quoteDescription(int resnumber)
{

    int		i;
    char	fname[FNAME_LEN];
    char	**file2Dim = NULL;

    sprintf(fname,"./file/%d",resnumber);
    file2Dim = readFile( fname );
    if( file2Dim == NULL ) {
        fatal_error("�� �R�����g��̔ԍ����w�肳��Ă��Ȃ����A�Y������L���͂���܂���B", body);
        return FALSE;
    }

    for( i = 0; *(file2Dim + i); i++ ) {
        if(i >= 15)
            printf(">%s", *(file2Dim + i));
    }

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid correctDescription(char **)
�����@�@�@�@�Fchar **value		�t�H�[�����瑗�M�����C����̓��e
�����@�@�@�@�G�t�H�[�����瑗�M���ꂽ�C����̓��e���Y������L���ԍ��ɏ������ށB
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int correctDescription(char **value)
{

    int		i;
    char	fname[BUFSIZE];					/*�t�@�C�����쐬�p*/
    FILE	*fp;						/*�L����������*/

    sprintf(fname,"./file/%d",atoi(*value));	/*path���܂߂��t�@�C�����쐬*/

    if((fp = fopen(fname,"w")) == NULL) {
        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 200)", body);
        return FALSE;
    }
    /*�L�����e���L���t�@�C���t�H�[�}�b�g�ɏ]���ď�������*/
    for(i = 0; i < 12; i++)
        fprintf(fp,"%s\n",*(value + i));/*[0-11]*/
    fprintf(fp,"\n\n\n");				/*[12-14]�c��̖��g�p�̈�̏o�́i���s�̂݁j*/
    fprintf(fp,"%s",*(value + 12));		/*[15]�R�����g*/
    fclose(fp);

    return TRUE;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint printDescription(int, FILE *, int)
�����@�@�@�@�Fint number			�\������L���ԍ�
�@�@�@�@�@�@�FFILE *fp			now�ւ̃t�@�C���|�C���^
�@�@�@�@�@�@�Fint old_number	�O��\�������ŏI�ԍ��i���ǈʒu�j
�����@�@�@�@�Fnumber�Ŏw�肳�ꂽ�L�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int printDescription(int number, FILE *fp, int old_number, CF config)
{

    int		i;
    unsigned int	brouser;
    char	buffer[BUFSIZE];
    char	comment[15][BUFSIZE];	/*���s�ǂݍ���*/

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
        "<!-- �L���ԍ�%05d�J�n -->\n"
        "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH=\"100%%\">\n"
        "<TR BGCOLOR=\"%s\">\n"
        "	<TD NOWRAP>\n"
        ,number, config.title_color);
    if(old_number != 0) {
        if(number > old_number) {
            printf("		 %s�� %04d�@%s\n�@",font,number,comment[1]);
        } else {
            printf("		 %s�� %04d�@%s\n�@",font,number,comment[1]);
        }
    } else {
        printf("		 %s%04d�@%s\n�@",font,number,comment[1]);
    }

    if(strstr(*(comment + 3),"@") != NULL)	/*���[���h���X��@���܂܂�Ă���ꍇ*/
        printf("<A HREF=\"mailto:%s\"><STRONG>%s</STRONG></A>\n",comment[3],comment[2]);
    else
        printf("<STRONG>%s</STRONG>\n",comment[2]);

    printf("�@�@<STRONG>%s</STRONG>�@�@\n",comment[5]);
    printf(
        "	</TD>\n"
        "	<TD NOWRAP WIDTH=\"100%%\">\n"
        "		<FORM METHOD=POST ACTION=\"./mewbbs.cgi\">\n"
        "		�@<INPUT TYPE=\"IMAGE\" SRC=\"./gif/res.gif\" ALT=\"#%d�ɃR�����g����\" BORDER=0>\n"
        ,number);

    if(atoi(comment[8]) != 0) {
        printf(
            "		<A HREF=\"./viewer.cgi?number=%d\" TARGET=\"_new\"><IMG SRC=\"./gif/fw.gif\" BORDER=0 ALT=\"�ԐM���L��(#%d)��\\��\"></A>\n"
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
    printUrl(*(comment + 11));	/*URL�o��*/

    printf(
        "</PRE>\n"
        "</BIG>\n"
        "</BLOCKQUOTE>\n"
        "<!--Remote Host: %s-->\n"
        "<!-- �L���ԍ�%05d�I�� -->\n\n"
        "<HR>\n\n"
        ,*(comment + 7) ,number);

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formConfirm(char **, char **)
�����@�@�@�@�Fchar **name	���M���ꂽ�t�H�[����NAME
�@�@�@�@�@�@�Fchar **value	���M���ꂽ�t�H�[����VALUE
�����@�@�@�@�F���e���e���m�F����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formConfirm(char **name, char **value, int count)
{

    int		i;		/*�J�E���^�p*/

    /*--- �m�F��ʏo�� ---*/
    printPageHeader("�o�^���e�̊m�F");
    puts(body);
    puts("<BR><TABLE BORDER=2 CELLPADDING=3 WIDTH=98%>");
    printf("<TR><TH COLSPAN=2>�o�^���e�̊m�F</TH></TR>");
    printf("<TR><TD WIDTH=20%% NOWRAP>�薼</TD><TD WIDTH=80%% NOWRAP>%s</TD></TR>",*value);

    if( strlen( *(value + 1) ) >= 1 )
        printf("<TR><TD VALIGN=TOP>���e</TD><TD><FONT SIZE=+1><PRE>%s</TD></TR>",*(value + 1));

    if(check_url(*(value + 2)) == 1)	/*URL�̓��͂�����*/
        printf("<TR><TD>�Q�ƃA�h���X</TD><TD>%s</TD></TR>",*(value + 2));

    printf("<TR><TD>���e��</TD><TD>%s</TD></TR>",*(value + 3));
    printf("<TR><TD>e-mail</TD><TD>%s</TD></TR>",*(value + 4));
    puts("</TABLE><FORM ACTION=\"mewbbs.cgi\" METHOD=\"POST\">");

    for(i = 0; *(name + i); i++) {
        if(strcmp("confirm", *(name + i)) && strcmp("write", *(name + i)))
            printf("<INPUT TYPE=\"HIDDEN\" NAME=\"%s\" VALUE=\"%s\">" ,*(name + i) ,*(value + i));
    }
    puts("<INPUT TYPE=\"HIDDEN\" NAME=\"confirm\" VALUE=\"1\">");
    puts("</CENTER><INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"�@�@�o�@�@�^�@�@\">");
    puts("</FORM>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formAdmin(char *)
�����@�@�@�@�Fchar *qspasswd		�t�H�[���ɓ��͂��ꂽ�p�X���[�h
�����@�@�@�@�F�Ǘ��҃��[�h�̃t�H�[�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formAdmin(char *qspasswd)
{
    DPRINTL(getenv("REMOTE_ADDR"));
    if(strcmp(master,qspasswd) != 0) {
        fatal_error("�� �p�X���[�h���Ԉ���Ă��܂��B������x�A���m���߂��������B", body);
        return FALSE;
    } else {
        printPageHeader("�Ǘ��҃��[�h");
        puts(body);
        /*--- �^�C�g���\�� ---*/
        puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");

        puts("<BR><BR>�� �Ǘ��҃��[�h�Ɉڍs���܂����B�s��������Ƃ�I�����Ă��������B<BR><BR>");
        puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>"
             "<TR><TD>"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fdel\" VALUE=\"�L���폜\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fundel\" VALUE=\"�L������\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fcorselect\" VALUE=\"�L���C��\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fchcfg\" VALUE=\"�ݒ�ύX\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fchpass\" VALUE=\"�p�X���[�h�ύX\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"fsecurity\" VALUE=\"�Z�L�����e�B�ݒ�\">"
             "<INPUT TYPE=\"SUBMIT\" NAME=\"next\" VALUE=\"�ʏ탂�[�h\">");
        printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
        puts("</TD></TR></TABLE></BODY></HTML>");
    }
    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formDelete(void)
�����@�@�@�@�F���݂̋L���ԍ����X�^�[�g�ƂƂ��āA�폜�p�^�C�g�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formDelete(int max)
{

    char	fname[BUFSIZE];			/*�t�@�C�����쐬*/
    char	*comment[15];		/*���e�\���p��Ɨ̈�*/
    int		i,j,k;				/*�J�E���^*/
    char	buffer[BUFSIZE];	/*�o�b�t�@*/
    FILE	*fpcomment;

    printPageHeader("�L���폜");
    puts(body);
    /*--- �^�C�g���\�� ---*/
    puts("<BR><BR>�� �L���̍폜���s���܂��B�폜�������L�����`�F�b�N���A�u�폜�v�������Ă��������B");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>");
    printf("<TR><TH WIDTH=1%% NOWRAP>�@</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>�ԍ�");
    puts("<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>���e����"
         "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>���e��"
         "<TH ALIGN=\"CENTER\">�^�C�g��");
    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%d",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*���s����菜��*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 240)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }
        /*--- �ʏ�L���i�V�K�E�R�����g�j ---*/
        if(atoi(*(comment + 10)) == 0) {
            if(atoi(*(comment + 8)) == 0) {	/*�V�K�L��*/
                printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            } else {	/*�R�����g�L��*/
                printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s"
                       "<TD NOWRAP>%s"
                       "<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            }
        } else if(atoi(*(comment + 10)) == 1) {	/*���e�ҍ폜*/
            printf("<TR><TD>�@");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        } else if(atoi(*(comment + 10)) == 2) {	/*�Ǘ��ҍ폜*/
            printf("<TR><TD>�@");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        }
        fclose(fpcomment);
        /*--- ��Ɨ̈��� ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"del\" VALUE=\"�@��@���@\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formUndelete(void)
�����@�@�@�@�G���݂̋L���ԍ����X�^�[�g�ƂƂ��āA�L�������p�^�C�g�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formUndelete(int max)
{

    char	fname[BUFSIZE];			/*�t�@�C�����쐬*/
    char	*comment[15];		/*���e�\���p��Ɨ̈�*/
    int		i,j,k;				/*�J�E���^*/
    char	buffer[BUFSIZE];	/*�o�b�t�@*/
    FILE	*fpcomment;

    printPageHeader("�L������");
    puts(body);
    /*--- �^�C�g���\�� ---*/
    puts("<BR><BR>�� �폜���ꂽ�L���̕������s���܂��B�����������L�����`�F�b�N���A�u�����v�������Ă��������B<BR>");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>");
    printf("<TR><TH WIDTH=1%% NOWRAP>�@</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>�ԍ�");
    puts("<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>���e����"
         "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>���e��"
         "<TH ALIGN=\"CENTER\">�^�C�g��");
    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%ddel",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*���s����菜��*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 250)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }

        printf("<TR><TD><INPUT TYPE=CHECKBOX NAME=%s VALUE=%s>",*comment,*comment);
        printf("<TD NOWRAP>%s"
               "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));

        fclose(fpcomment);
        /*--- ��Ɨ̈��� ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"undel\" VALUE=\"�@���@���@\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formCorSelect(void)
�����@�@�@�@�G���݂̋L���ԍ����X�^�[�g�ƂƂ��āA�L���C���p�^�C�g�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formCorSelect(int max)
{

    char	fname[BUFSIZE];			/*�t�@�C�����쐬*/
    char	*comment[15];		/*���e�\���p��Ɨ̈�*/
    int		i,j,k;				/*�J�E���^*/
    char	buffer[BUFSIZE];	/*�o�b�t�@*/
    FILE	*fpcomment;

    printPageHeader("�L���C��");
    puts(body);

    /*--- �^�C�g���\�� ---*/
    printf("<BR><BR>\n"
           "�� �L���̏C�����s���܂��B�C���������L�����`�F�b�N���A�u�C���v�������Ă��������B<BR>\n"
           "<FORM METHOD=POST ACTION=\"mewbbs.cgi\">\n"
           "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=80%%>"
           "<TR><TH WIDTH=1%% NOWRAP>�@</TH><TH ALIGN=\"CENTER\" NOWRAP WIDTH=5%%>�ԍ�"
           "<TH ALIGN=\"CENTER\" WIDTH=10%% NOWRAP>���e����"
           "<TH ALIGN=\"CENTER\" WIDTH=20%% NOWRAP>���e��"
           "<TH ALIGN=\"CENTER\">�^�C�g��");

    for(j = now; j > (now - max); j--) {
        sprintf(fname,"./file/%d",j);
        if((fpcomment = fopen(fname,"r")) == NULL)
            continue;
        for(i = 0; i < 11; i++) {
            if(fgets(buffer,BUFSIZE,fpcomment) == NULL)
                break;
            removeNewline(buffer);	/*���s����菜��*/
            if((*(comment + i) = (char *)malloc(strlen(buffer) + 1)) == NULL) {
                fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 260)", body);
                return FALSE;
            }
            strcpy(*(comment + i),buffer);
        }
        /*--- �ʏ�L���i�V�K�E�R�����g�j ---*/
        if(atoi(*(comment + 10)) == 0) {
            if(atoi(*(comment + 8)) == 0) {	/*�V�K�L��*/
                printf("<TR><TD><INPUT TYPE=RADIO NAME=num VALUE=%s>",*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            } else {	/*�R�����g�L��*/
                printf("<TR><TD><INPUT TYPE=RADIO NAME=num VALUE=%s>",*comment);
                printf("<TD NOWRAP>%s"
                       "<TD NOWRAP><FONT SIZE=-1>%s"
                       "<TD NOWRAP>%s"
                       "<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
            }
        } else if(atoi(*(comment + 10)) == 1) {	/*���e�ҍ폜*/
            printf("<TR><TD>�@");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        } else if(atoi(*(comment + 10)) == 2) {	/*�Ǘ��ҍ폜*/
            printf("<TR><TD>�@");
            printf("<TD NOWRAP>%s"
                   "<TD NOWRAP><FONT SIZE=-1>%s<TD NOWRAP>%s<TD>%s",*comment,*(comment + 1),*(comment + 2),*(comment + 5));
        }
        fclose(fpcomment);
        /*--- ��Ɨ̈��� ---*/
        for(k=11 ; k < i ; k++) {
            if(*(comment + k))
                free(*(comment + k));
        }
    }

    puts("<TR><TD COLSPAN=5><INPUT TYPE=\"SUBMIT\" NAME=\"fcorinput\" VALUE=\"�@�C�@���@\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formCorInput(int)
�����@�@�@�@�Fint resnumber		�C���Ώۂ̋L���ԍ�
�����@�@�@�@�Gresnumber�Ŏw�肳�ꂽ�ԍ��̏C���p�t�H�[�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formCorInput(int resnumber)
{

    int		i;
    char	**comment = NULL;
    char	fname[BUFSIZE];

    if (resnumber == 0) {
        fatal_error("�� ���e�C������L����I�����Ă��������B", body);
        return FALSE;
    } else if(resnumber > 0) {
        sprintf(fname,"./file/%d",resnumber);
        comment = readFile( fname );
        if( comment == NULL ) {
            return FALSE;
        } else {
            for(i = 0; *(comment + i) && i < 15; i++)
                removeNewline( *(comment + i) );	/*���s����菜��*/
        }
    }
    printPageHeader("�w��ԍ��̋L���C��");
    puts(body);
    printf("<BR><BR>�� %d�Ԃ̏C�����s���܂��B�L�����C�����u�C���v�������Ă��������B<BR>",resnumber);

    puts("<FORM METHOD=\"POST\" ACTION=\"mewbbs.cgi\">\n<TABLE BORDER=1>\n");

    printf("<INPUT TYPE=HIDDEN NAME=number VALUE=\"%s\">",*comment);
    printf("<INPUT TYPE=HIDDEN NAME=time VALUE=\"%s\">",*(comment + 1));
    puts("<TR><TD><STRONG>���e��</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=writer VALUE=\"%s\" SIZE=20 MAXLENGTH=30></TD></TR>",*(comment + 2));
    puts("<TR><TD><STRONG>e-mail</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=mail VALUE=\"%s\" SIZE=20 MAXLENGTH=150></TD></TR>",*(comment + 3));
    printf("<INPUT TYPE=HIDDEN NAME=comcount VALUE=\"%s\">",*(comment + 4));
    puts("<TR><TD><STRONG>�^�C�g��</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=title VALUE=\"%s\" SIZE=40 MAXLENGTH=40></TD></TR>",*(comment + 5));
    printf("<INPUT TYPE=HIDDEN NAME=useragent VALUE=\"%s\">",*(comment + 6));
    printf("<INPUT TYPE=HIDDEN NAME=remotehost VALUE=\"%s\">",*(comment + 7));
    printf("<INPUT TYPE=HIDDEN NAME=comnumber VALUE=\"%s\">",*(comment + 8));
    printf("<INPUT TYPE=HIDDEN NAME=comwriter VALUE=\"%s\">",*(comment + 9));
    printf("<INPUT TYPE=HIDDEN NAME=atribute VALUE=\"%s\">",*(comment + 10));
    puts("<TR><TD><STRONG>�Q�ƃA�h���X</STRONG></TD>");
    printf("<TD><INPUT TYPE=\"TEXT\" NAME=url VALUE=\"%s\" SIZE=60 MAXLENGTH=300></TD></TR>",*(comment + 11));
    puts("<TR><TD VALIGN=\"TOP\"><STRONG>���e</STRONG></TD>");
    puts("<TD><TEXTAREA NAME=\"comment\" ROWS=8 COLS=60>");
    for(i = 15; *(comment + i); i++)
        fputs(*(comment + i),stdout);

    printf(
        "</TEXTAREA></TD></TR></TABLE>"
        "<TABLE BORDER=1>"
        "<TR><TD COLSPAN=2><INPUT TYPE=\"SUBMIT\" NAME=\"correction\" VALUE=\"�@�C�@���@\">"
    );
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    puts("<INPUT TYPE=\"SUBMIT\" NAME=\"fcorselect\" VALUE=\"�@�߁@��@\">");
    puts("</TD></TR></TABLE>");

    freeTwoDimArray( comment );

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formConfig(void)
�����@�@�@�@�G�ݒ�ύX�ׂ̈̃t�H�[�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formConfig(CF config)
{

    printPageHeader("�ݒ�ύX");
    puts(body);
    printf("<BR><BR>�� �ݒ�̕ύX���s���܂��B�ύX���������ڂ���͂��u�ύX�v�������Ă��������B�i<A HREF=\"%s#CONFIG\" TARGET=\"_new\">�w���v</A>�j<BR>", ADMIN_HELP_URL);
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\">");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>");
    printf("<TR><TD NOWRAP><STRONG>�E�C���h�E�^�C�g��</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"APTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.aptitle);
    printf("<TR><TD NOWRAP><STRONG>���C���^�C�g��</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"MAINTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.maintitle);
    printf("<TR><TD NOWRAP><STRONG>�T�u�^�C�g��</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"SUBTITL\" VALUE=\"%s\" SIZE=30 MAXLENGTH=200></TD></TR>",config.subtitle);
    printf("<TR><TD NOWRAP><STRONG>�\\���L����(1-50)</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"max\" VALUE=\"%d\" SIZE=2 MAXLENGTH=2></TD></TR>",config.max);
    if(config.tag == 1)
        printf("<TR><TD NOWRAP><STRONG>HTML�^�O</STRONG></TD><TD><SELECT NAME=\"TAG\"><OPTION VALUE=1>��<OPTION VALUE=0>�~</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>HTML�^�O</STRONG></TD><TD><SELECT NAME=\"TAG\"><OPTION VALUE=0>�~<OPTION VALUE=1>��</SELECT></TD></TR>");

    if(config.flag_write == 0)
        printf("<TR><TD NOWRAP><STRONG>�������݋���</STRONG></TD><TD><SELECT NAME=\"WRITE\"><OPTION VALUE=0>��<OPTION VALUE=1>�~</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>�������݋���</STRONG></TD><TD><SELECT NAME=\"WRITE\"><OPTION VALUE=1>�~<OPTION VALUE=0>��</SELECT></TD></TR>");

    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�w�i</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"BACKGROUND\" VALUE=\"%s\" SIZE=50 MAXLENGTH=200></TD></TR>",CC_URL ,config.background);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�w�i�F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"BGCOLOR\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL ,config.bgcolor);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�����F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"TEXT\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.text);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�����N�����F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"LINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.link_color);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�������N�����F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"VLINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.vlink);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�����N�������F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"ALINK\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.alink);
    printf("<TR><TD NOWRAP><A HREF=\"%s\" TARGET=\"_new\"><STRONG>�L���^�C�g���w�i�F</STRONG></A></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"TCOLOR\" VALUE=\"%s\" SIZE=10 MAXLENGTH=20></TD></TR>",CC_URL,config.title_color);
    if(config.fweight == 0)
        printf("<TR><TD NOWRAP><STRONG>�����̑���</STRONG></TD><TD><SELECT NAME=\"FWEIGHT\"><OPTION VALUE=0>�ʏ�<OPTION VALUE=1>����</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>�����̑���</STRONG></TD><TD><SELECT NAME=\"FWEIGHT\"><OPTION VALUE=1>����<OPTION VALUE=0>�ʏ�</SELECT></TD></TR>");

    if(config.fsize == 2)
        printf("<TR><TD NOWRAP><STRONG>�����̑傫��</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=2>������<OPTION VALUE=3>�ʏ�<OPTION VALUE=4>�傫��</SELECT></TD></TR>");
    else if(config.fsize == 4)
        printf("<TR><TD NOWRAP><STRONG>�����̑傫��</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=4>�傫��<OPTION VALUE=3>�ʏ�<OPTION VALUE=2>������</SELECT></TD></TR>");
    else
        printf("<TR><TD NOWRAP><STRONG>�����̑傫��</STRONG></TD><TD><SELECT NAME=\"FSIZE\"><OPTION VALUE=3>�ʏ�<OPTION VALUE=2>������<OPTION VALUE=4>�傫��</SELECT></TD></TR>");

    printf("<TR><TD NOWRAP><STRONG>�ő�o�^��</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"REGMAX\" VALUE=\"%d\" SIZE=5 MAXLENGTH=10></TD></TR>",config.regmax);
    printf("<TR><TD NOWRAP><STRONG>�z�[���y�[�W</STRONG></TD><TD><INPUT TYPE=\"TEXT\" NAME=\"HOME_URL\" VALUE=\"%s\" SIZE=50 MAXLENGTH=200></TD></TR>",config.home_url);

    puts("<TR><TD COLSPAN=2 NOWRAP><INPUT TYPE=\"SUBMIT\" NAME=\"chcfg\" VALUE=\"�@�ρ@�X�@\">"
         "<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\">",master);
    puts("</TD></TR></TABLE></FORM></BODY></HTML>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid formChpass(void)
�����@�@�@�@�G�p�X���[�h�ύX�p�̃t�H�[�����o�͂���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void formChpass(void)
{

    printPageHeader("�p�X���[�h�ύX");
    puts(body);
    printf(
        "<BR><BR>\n"
        "�� �p�X���[�h�̕ύX���s���܂��B�ύX����p�X���[�h����͂��A�u�ύX�v�������Ă��������B\n"
        "<BR><BR>\n"
        "<FORM METHOD=POST ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>\n"
        "<TR>\n"
        "	<TD NOWRAP>\n"
        "		<STRONG>�V�p�X���[�h</STRONG>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"PASSWORD\" NAME=\"NEW_PASS\" VALUE=\"\" SIZE=8 MAXLENGTH=8>\n"
        "	</TD>\n"
        "<TR>\n"
        "	<TD NOWRAP>\n"
        "		<STRONG>�m�F</STRONG>\n"
        "	</TD>\n"
        "	<TD>\n"
        "		<INPUT TYPE=\"PASSWORD\" NAME=\"CF_PASS\" VALUE=\"\" SIZE=8 MAXLENGTH=8>\n"
        "	</TD>\n"
        "</TR>\n"
        "<TR>\n"
        "	<TD COLSPAN=2>\n"
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"chpass\" VALUE=\"�@�ρ@�X�@\">"
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">"
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
�v���g�^�C�v�Fvoid formSecurity(void)
�����@�@�@�@�G.denied���瓊�e�֎~�z�X�g��ǂݍ��ݕ\������B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int formSecurity(int proxy, int hostchk)
{

    int		i;				/*�J�E���^*/
    char	buffer[BUFSIZE];	/*�o�b�t�@*/
    FILE	*fp;

    printPageHeader("�Z�L�����e�B�ݒ�");
    puts(body);

    printf(
        "<!-- �Z�L�����e�B�֘A�̐ݒ�ύX�t�H�[�� -->\n"
        "<BR><BR>\n"
        "�� �Z�L�����e�B�֘A�̐ݒ���s���܂��B�i<A HREF=\"%s#SECURITY\" TARGET=\"_new\">�w���v</A>�j", ADMIN_HELP_URL);
    printf(
        "<BR>\n<HR>\n<BR>\n"
        "<BLOCKQUOTE>\n"
        "�� ���e�֎~�z�X�g�̒ǉ����s���܂��B\n"
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
        "		<INPUT TYPE=\"SUBMIT\" NAME=\"\" VALUE=\"�@�ǁ@���@\">\n"
        "	</TD>\n"
        "</TR>\n"
        "</TABLE>\n"
        "</FORM>\n"
        "</BLOCKQUOTE>\n"
        "<HR>\n"
    );


    /*DENIED���X�g�̓ǂݍ��݂ƕ\��*/
    if((fp = fopen(DENIED_LIST,"r")) == NULL) {
        fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 301)", body);
        return FALSE;
    } else {
        if(fgets(buffer,BUFSIZE,fp) != NULL) {

            rewind( fp );

            printf(
                "<BR>\n"
                "<BLOCKQUOTE>\n"
                "�� ���e�֎~�z�X�g�̍폜���s���܂��B\n"
                "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
                "<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3 WIDTH=\"50%%\">\n"
            );

            for(i = 1; ; i++) {
                if(fgets(buffer,BUFSIZE,fp) == NULL)
                    break;
                if(strlen(buffer) > 1) {
                    removeNewline(buffer);	/*���s����菜��*/
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
                "	<INPUT TYPE=\"SUBMIT\" NAME=\"delhosts\" VALUE=\"�@��@���@\">\n"
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
        "<!-- ���̑��̃Z�L�����e�B�ݒ�ύX�t�H�[�� -->\n"
        "<BR>\n"
        "<BLOCKQUOTE>\n"
        "�� ���̑��̃Z�L�����e�B�ݒ���s���܂��B\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">\n"
        "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=3 WIDTH=\"80%%\">\n"
    );

    printf(
        "<TR>\n"
        "<TD NOWRAP WIDTH=\"40%%\">\n"
        "	�EPROXY�o�R���e"
        "</TD>"
    );
    if(proxy == 1) {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=1>PROXY�o�R�̓��e�֎~</OPTION>\n"
            "	<OPTION VALUE=0>�S�Ẵz�X�g���瓊�e����</OPTION>\n"
            "	<OPTION VALUE=2>����PROXY�o�R�̓��e�̂݋֎~</OPTION>\n"
            "	</SELECT>\n"
            "</TD>\n"
        );
    } else if(proxy == 2) {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=2>����PROXY�o�R�̓��e�̂݋֎~</OPTION>\n"
            "	<OPTION VALUE=0>�S�Ẵz�X�g���瓊�e����</OPTION>\n"
            "	<OPTION VALUE=1>PROXY�o�R�̓��e�֎~</OPTION>\n"
            "	</SELECT>\n"
            "</TD>\n"
        );
    } else {
        printf(
            "<TD NOWRAP WIDTH=\"60%%\">\n"
            "	<SELECT NAME=\"PROXY\">\n"
            "	<OPTION VALUE=0>�S�Ẵz�X�g���瓊�e����</OPTION>\n"
            "	<OPTION VALUE=1>PROXY�o�R�̓��e�֎~</OPTION>\n"
            "	<OPTION VALUE=2>����PROXY�o�R�̓��e�̂݋֎~</OPTION>\n"
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
        "	�E�s���ȃz�X�g����̓��e"
        "</TD>"
    );
    if(hostchk == 1) {
        printf(
            "<TD NOWRAP>\n"
            "	<INPUT TYPE=\"CHECKBOX\" NAME=\"HOSTCHK\" VALUE=1 CHECKED>\n"
            "	���ۂ���\n"
            "</TD>\n"
        );
    } else {
        printf(
            "<TD NOWRAP>\n"
            "	<INPUT TYPE=\"CHECKBOX\" NAME=\"HOSTCHK\" VALUE=1>\n"
            "	���ۂ���\n"
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
        "	<INPUT TYPE=\"SUBMIT\" NAME=\"chsec\" VALUE=\"�@�ρ@�X�@\">\n"
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
        "	<INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">\n"
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
�v���g�^�C�v�Fvoid chPass(char **value)
�����@�@�@�@�Fchar **value		�t�H�[�����瑗���Ă����p�X���[�h
�����@�@�@�@�G�t�H�[�����瑗�M���ꂽ�V�p�X���[�h���m�F�p�X���[�h��F�؂��A
�@�@�@�@�@�@�@��������΁A�V�p�X���[�h���Í�����passwd�ɏ������ށB
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int chPass(char **value)
{

    FILE *fp;
    char *passwd;

    if(strcmp(*value, *(value + 1))) {
        fatal_error("�� �V�p�X���[�h�Ɗm�F�p�X���[�h�̓��͂��قȂ��Ă��܂��B������x���m���߂��������B", body);
        return FALSE;
    }
    if((fp = fopen(PASSWD_PATH,"w")) == NULL) {
        fatal_error("�� �p�X���[�h�t�@�C�������݂��Ȃ����Aother�ɏ������݌������^�����Ă��܂���B", body);
        return FALSE;
    }
    passwd = crypt(*value,SALT);
    fprintf(fp,"%s",passwd);
    fclose(fp);
    printPageHeader("�p�X���[�h�ύX�����I");
    puts(body);
    puts("<BR><BR>�� �p�X���[�h���ύX����܂����B�p�X���[�h�̊Ǘ��ɂ͏\\�����ӂ��Ă��������B");
    puts("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=3>");
    puts("<FORM METHOD=POST ACTION=\"mewbbs.cgi\"><INPUT TYPE=\"SUBMIT\" NAME=\"fadmin\" VALUE=\"�@�߁@��@\">");
    printf("<INPUT TYPE=HIDDEN NAME=\"now\" VALUE=%d>",now);
    printf("<INPUT TYPE=HIDDEN NAME=\"pass\" VALUE=\"%s\"></TD></TR></TABLE></BODY></HTML>",passwd);

    return TRUE;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid getPass(void)
�����@�@�@�@�Gpasswd����p�X���[�h���擾���A�Í�������master�ɃZ�b�g����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getPass(void)
{

    FILE *fp;

    if((fp = fopen(PASSWD_PATH,"r")) == NULL) {
        fatal_error("�� �p�X���[�h�t�@�C�������݂��Ȃ����Aother�ɓǂݍ��݌������^�����Ă��܂���B", body);
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

    struct tm	*jst;	/*sys/time.h��tm�\���́i���Ԏ擾�j*/
    time_t		ti;
    static char	expire[BUFSIZE];
    unsigned int year;

    ti = time(NULL);
    ti += 9*60*60;		/*GMT��JST*/

    ti += (86400*days);
    jst = gmtime(&ti);

    if( 0 <= jst->tm_year && jst->tm_year < 70 )		/*2000�N���΍�*/
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

    int		start,end;						/*���̃y�[�W�\��*/
    int		resnumber = 0;					/*�R�����g�Ώۂ̋L���ԍ�*/
    int		comnumber = 0;					/*�R�����g��*/
    int		chkbox;							/*�`�F�b�N�{�b�N�X�p�̃t���O*/
    char	*title = NULL;		/*�L���^�C�g���ւ̃|�C���^*/
    char	writer[HANDLE_MAXSIZE];			/*���e��*/
    char	pass[PASSWD_LEN];				/*���e�҂̍폜�p�p�X���[�h�ւ̃|�C���^*/
    int		number;							/*�L���ԍ�*/
    char	*temp;
    int		i,j,l;							/*�J�E���^�p*/
    char	fname[BUFSIZE];					/*�t�@�C���l�[���쐬*/
    FILE	**fp,*fp_read;					/*�L���\���E�L���������݋���*/
    char	*method;						/*REQUEST_METHOD*/
    char	**name, **value;				/*�t�H�[���f�[�^�擾�p*/
    int		count = -2;						/*�t�H�[���f�[�^��name=value�̐�*/
    char	*handle = NULL,*address = NULL,*passwd = NULL;		/*�N�b�L�[�̓��e�i�[*/
    char	*type;							/*�^�C�v����*/
    int		ctype = 0;						/*confirm�̓���t���O*/
    int		old_number;						/*�O��̊��ǈʒu*/
    int		adflag = 0;						/*�Ǘ��҃��[�h�֓���Ƃ��̃t���O*/
    char	buffer[BUFSIZE];						/*�o�b�t�@*/
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

    number = getCountInt(COUNTER_FILE);	/*���݂̋L���ԍ��擾*/
    getCookieUser(&chkbox, &handle, &address, &passwd);		/*�N�b�L�[�擾*/
    old_number = getCookieNew();					/*�N�b�L�[�擾*/

    /*----- REQUEST_METHOD��[POST]�̎�-----*/
    if(strcmp(method,"POST") == 0) {

        DPRINTL("Start: POST Method.");

        if(!checkReferer())
            return 1;

        count = getForm(&name, &value);		/*stdout���珑�����ݓ��e���擾*/
        if(count == -2 || count == -1) {
            fatal_error("�� �t�H�[���f�[�^�̎擾�Ɏ��s���܂����B", body);
            freedata(name, value);
            return 1;
        }
        if((type = getOption(name)) == NULL) {
            fatal_error("�� �����ȃI�v�V�����ł��B", body);
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
            /*�Ǘ��҃��[�h�ɓ���Ƃ��͓��͂��ꂽ�p�X���[�h���Í������Apasswd����Í������ꂽ�p�X���[�h��ǂݍ��ݔF�؂���B
            �ʏ펞�͊Ǘ��҃��[�h�ɓ��鎞�ɈÍ������ꂽ�p�X���[�h���t�H�[���ɕۑ����Ă����A��������Ƃ���ǂݏo���Ă���̂�
            ���̂܂ܗ��p���F�؂���Badflag�͊Ǘ��҃��[�h�ɓ���Ƃ��ɗp����t���O�B*/
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
        if(!strcmp(type,"write")) {		/*�L���o�^*/
            if(config.flag_write == 1) { /*�������݂�������Ă���Ƃ�*/
                fatal_error( "�� �������܂����A�������܋L���̓o�^�𒆎~�����Ă��������Ă���܂��B", body);
                return 1;
            }

            if(config.tag == 0 || ctype != 0) {
                for(i = 0; *(value + i); i++) {
                    if(config.tag == 0) {
                        temp = replacestring(*(value + i),"<","&lt;");				/*�^�O�𖳌�������*/
                        if( temp != NULL ) {
                            free(*(value + i));
                            *(value + i) = temp;
                        }
                    }
                    if(ctype == 2) {
                        temp = replacestring(*(value + i),(char*)"\"",(char*)"&#34;");	/*"�����̎Q��(&#34;)��*/
                        if( temp != NULL ) {
                            free(*(value + i));
                            *(value + i) = temp;
                        }
                    }
                }
            }

            /*--- �t�H�[���ɓ��͗��`�F�b�N ---*/
            if(**value == '\0') { /* �^�C�g���̓��͂��Ȃ��ꍇ */
                free(*value);
                *value = (char *)malloc( strlen( untitled ) + 1 );
                strcpy( *value, untitled );
            }

            if(!strcmp(strsub(buffer, *(value + 3), 0, 1), " ")) {
                fatal_error("�� �n���h���̂͂��߂ɋ󔒂͎g���܂���B������x�A���m���߂��������B", body);
                freedata(name, value);
                return 1;
            }
            if(!strcmp(strsub(buffer, *(value + 3), 0, 2), "�@")) {
                fatal_error("�� �n���h���̂͂��߂ɋ󔒂͎g���܂���B������x�A���m���߂��������B", body);
                freedata(name, value);
                return 1;
            }

            if(**(value + 3) == '\0') {	/* �n���h���̓��͂��Ȃ��ꍇ */
                fatal_error("�� �n���h���̓��͕͂K�{�ł��B������x�A���m���߂��������B", body);
                freedata(name, value);
                return 1;
            }
            if(check_url(*(value + 2)) == 0) {
                fatal_error("�� URL�̌`���`�����Ⴂ�܂��B������x�A���m���߂��������B", body);
                exit(1);
            }

            temp = replacestring(*value ,"<","&lt;");				/*�^�C�g���̃^�O�𖳌�������*/
            if( temp != NULL ) {
                free(*value);
                *value = temp;
            }
            /*--- �o�^�m�F�t�H�[���o�� ---*/
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
                fatal_error("�� �J�E���^�[�t�@�C���ɏ������݌������^�����Ă��܂���B", body);
                return 1;
            }
            limitDescription(number, config.regmax);


            free(handle);
            free(address);
            handle = *(value + 3);
            address = *(value + 4);

        } else if(!strcmp(type,"reload")) {			/*�X�V*/
            ;
        } else if(!strcmp(type,"home")) {				/*�f���I��*/
            printf("Location: %s\n\n" ,config.home_url);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"del")) {				/*�L���폜*/
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
        } else if(!strcmp(type,"fdel")) {			/*�L���폜�p�t�H�[��*/
            if(!formDelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fchcfg")) {			/*�ݒ�ύX�p�t�H�[��*/
            formConfig(config);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"chcfg")) {			/*�ݒ�ύX*/
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
        } else if(!strcmp(type,"chsec")) {			/*PROXY�o�R���e�֎~�ݒ�ύX*/
            chSecurity(name, value, count);
            if(!getConfig(&config)) {
                freedata(name, value);
                return 1;
            }
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fundel")) {		/*�L�������p�t�H�[��*/
            if(!formUndelete(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fadmin")) {			/*�Ǘ��҃��[�h�t�H�[��*/
            if(!getConfig(&config))
                return 1;
            if(!formAdmin(pass)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"undel")) {			/*�L������*/
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
        } else if(!strcmp(type,"fcorselect")) {			/*�L���C���p�ꗗ*/
            if(!formCorSelect(config.max)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"fcorinput")) {			/*�L���C���p�t�H�[��*/
            if(!formCorInput(resnumber)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"correction")) {		/*�L���C��*/
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
        } else if(!strcmp(type,"fchpass")) {			/*�p�X���[�h�ύX�p�t�H�[��*/
            formChpass();
            freedata(name, value);
            return 0;
        } else if(!strcmp(type,"chpass")) {			/*�p�X���[�h�ύX*/
            if(!chPass(value)) {
                freedata(name, value);
                return 1;
            }
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "fsecurity")) {			/*���e�֎~�z�X�g�ǉ��E�폜�t�H�[��*/
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "delhosts")) {		/*���e�֎~�z�X�g�폜*/
            if(!deleteHosts( name, value )) {
                freedata(name, value);
                return 1;
            }
            formSecurity(config.proxy, config.hostchk);
            freedata(name, value);
            return 0;
        } else if(!strcmp(type, "addhost")) {			/*���e�֎~�z�X�g�ǉ�*/
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
        } else if(!strcmp(type,"res")) {				/*�R�����g*/
            if(resnumber > 0) {
                sprintf(fname,"./file/%d",resnumber);

                if((fp_read = fopen(fname,"r")) == NULL) {
                    fatal_error("�� �R�����g��̔ԍ����w�肳��Ă��Ȃ����A�Y������L���͂���܂���B", body);
                    return 1;
                }
                for( i = 0; i < 6; i++ ) {
                    if(fgets(buffer,BUFSIZE,fp_read) == NULL) {
                        if(ferror(fp_read)) {
                            fatal_error("�� ���炩�̃G���[���������܂����B<BR>�����悤�ȃG���[�����x���o��ꍇ�͊Ǘ��҂ɘA�����Ă��������B(error code 340)", body);
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
                            fatal_error( "�� �V�X�e���G���[�ł��B�ēx���s���Ă��������B", body );
                    }
                }
                fclose(fp_read);
            }
        }
    } else {
        type = "Unknown";
    }
    /*�\������L���̐��ݒ�i���݂̋L����-max���P�ȉ��������ꍇ�͂O�ɐݒ�j*/

    DPRINTL("Start: GET Method.");

    if(!strcmp(type,"next")) {	/*���̃y�[�W��\��*/
        start = now;
        end = start - config.max;
        if(end < 1)
            end = 0;
    } else {	/*�ʏ�*/
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
        "\n<!-- ���e�t�H�[�� -->\n"
        "<FORM METHOD=\"POST\" ACTION=\"./mewbbs.cgi\">"
        "<TABLE BORDER=3 CELLPADDING=3 CELLSPACING=3>\n"
    );

    if(resnumber != 0) {
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>�薼</STRONG>\n"
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
            "		<STRONG>���e</STRONG>\n"
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
            "		<STRONG>�Q�ƃA�h���X</STRONG>\n"
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
            "		<STRONG>���O</STRONG>\n"
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
                "		<STRONG>�o�^���e���m�F</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        } else {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"checkbox\" NAME=\"confirm\" VALUE=\"2\">\n"
                "		<STRONG>�o�^���e���m�F</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        }
        printf(
            "<TR>\n"
            "	<TD COLSPAN=2>\n"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"�L��#%d�֕ԐM\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"reload\" VALUE=\"���� �^ �X�V\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">"
            "	</TD>\n"
            "</TR>\n"
            ,resnumber
        );
    } else {
        printf(
            "<TR>\n"
            "	<TD NOWRAP>\n"
            "		<STRONG>�薼</STRONG>\n"
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
            "		<STRONG>���e</STRONG>\n"
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
            "		<STRONG>�Q�ƃA�h���X</STRONG>\n"
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
            "		<STRONG>���O</STRONG>\n"
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
            "		<INPUT TYPE=\"HIDDEN\" NAME=\"resnum\" VALUE=\"�����\">\n"
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
                "		<STRONG>�o�^���e���m�F</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        } else {
            printf(
                "<TR>\n"
                "	<TD COLSPAN=2>\n"
                "		<INPUT TYPE=\"CHECKBOX\" NAME=\"confirm\" VALUE=\"2\">\n"
                "		<STRONG>�o�^���e���m�F</STRONG>\n"
                "	</TD>\n"
                "</TR>\n"
            );
        }
        printf(
            "<TR>\n"
            "	<TD COLSPAN=2>\n"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"write\" VALUE=\"���@�e  ��  ��\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"reload\" VALUE=\"���� �^ �X�V\">"
            "<INPUT TYPE=\"SUBMIT\" NAME=\"home\" VALUE=\"�z�[���y�[�W��\">\n"
            "	</TD>\n"
            "</TR>\n"
        );
    }
    printf("</TABLE>\n</FORM>\n");
    printf("\n<!-- �X�e�[�^�X�\\�� -->\n");
    printStatus(start, end, number, old_number, type, value, config);	/*�X�e�[�^�X�\��*/
    printf("<HR SIZE=3>\n\n");

    /*--- �L����\�� ---*/
    fp = (FILE **)malloc(sizeof(FILE *) * config.max);
    for(j = start,l = 0 ; j > end; j--, l++) {	/*�t�@�C���|�C���^���܂Ƃ߂ăI�[�v��*/
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
    for(j = start,l = 0 ; j > end; j--, l++) {	/*�t�@�C���|�C���^���܂Ƃ߂ăN���[�Y*/
        if(*(fp + l) == NULL)
            continue;
        fclose(*(fp + l));
    }
    free(fp);
    /*--- �L���\\���I��---*/

    printf("\n<!-- �R���g���[���t�H�[���\\�� -->\n");
    printForm2(start, end, config.regmax, config.max, old_number, number);	/*�L���\\���I����̃t�H�[���o��*/
    puts("\n<HR>");

    /*----- ���쌠�\���i�폜���Ȃ��ł��������j----- */
    puts(
        "\n<!-- ���쌠�\\�� -->\n"
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

    return 0;	/*--- �I�� ---*/
}
/*End Of File*/
