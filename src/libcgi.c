/*---------------------------------------------------------------------------
 * libcgi.c Ver4.2
 * CGI�ėp���[�e�B���e�B
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
�v���g�^�C�v�Fint selectBrouser( void );
�Ԓl		�Fint ... Internet Explorer - 1
			�@        Netscape Navegator - 2
			�@        �G���[ or ���̑� - 0
�����@�@�@�@�F�g�p���Ă���u���E�U�𔻕ʂ���
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
�v���g�^�C�v�Fvoid decode(char *)
�����@�@�@�@�Fchar *url		�G���R�[�h���ꂽ������
�����@�@�@�@�Furl���f�R�[�h����B
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
�v���g�^�C�v�Fint getdata (char ***, char ***)
�����@�@�@�@�Fchar ***dname		�t�H�[���f�[�^��name���m�ۂ��邽�߂̗̈�ւ̃|�C���^
�@�@�@�@�@�@�@char ***dvalue	�t�H�[���f�[�^��value���m�ۂ��邽�߂̗̈�ւ̃|�C���^
�߂�l�@�@�@�Fint				name=vallue�̑g��
�����@�@�@�@�F�t�H�[���̃f�[�^���󂯎��Aname��value�ɕ�������B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *getFormData( void )
{

    char *qstring = NULL;			/*stdin data*/
    char *qs;
    char *method;			/*REQUEST_METHOD*/
    int ictl;				/*CONTENT_LENGTH*/

    method = getenv("REQUEST_METHOD");	/*�����𒲂ׂ�*/
    if(method == NULL) {
        return NULL;
    } else if(!strcmp(method, "POST")) {	/*POST�̎�*/

        ictl = atoi(getenv("CONTENT_LENGTH"));
        if(ictl == 0)	/*�󂯎��f�[�^�����݂��Ȃ�*/
            return NULL;
        if((qstring = (char *)malloc( sizeof(char) * (ictl + 1))) == NULL)
            return NULL;
        memset( (char *)qstring, '\0', sizeof(qstring) );
        if((fread(qstring, ictl, 1, stdin)) != 1)
            return NULL;
        qstring[ictl] = '\0';

    } else if(!strcmp(method, "GET")) {			/*GET�̎�*/
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
�v���g�^�C�v�Fint freedata (char **, char **, int)
�����@�@�@�@�Fchar **name	����������Ώۂ̓񎟔z��
�@�@�@�@�@�@�@char **value	����������Ώۂ̓񎟔z��
�@�@�@�@�@�@�@int count		name=value�̑g��
�����@�@�@�@�Fname��value�̃��������������B
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
    int				flag = 1;		/*strspl()�ŗp����t���O*/
    unsigned int	record_num;

    http_cookie = getenv( "HTTP_COOKIE" );
    if( http_cookie == NULL )	/*HTTP_COOKIE�����݂��Ȃ��ꍇ*/
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
    *(tmp + record_num) = NULL;		/*�I�[��NULL*/
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

    int count;				/*ddname=dvalue�̑g��*/
    char **tmpname = NULL,**tmpval = NULL;	/*dname��dvalue���i�[����Q���z��*/
    int flag = 1;

    /*----- �������f�R�[�h -----*/
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
�v���g�^�C�v�Fint getcookiesdata (char ***, char ***, char *)
�����@�@�@�@�Fchar ***cname		cookie��name���m�ۂ��邽�߂̗̈�ւ̃|�C���^
�@�@�@�@�@�@�@char ***cvalue	cookie��name���m�ۂ��邽�߂̗̈�ւ̃|�C���^
�@�@�@�@�@�@�@char *name		�擾����cookie�̖��O
�߂�l�@�@�@�Fint				name=value�̑g��
�����@�@�@�@�Fname�Ŏw�肳�ꂽcookie��name��value�𕪊����i�[����B
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
�v���g�^�C�v�Fvoid fatal_error (const char *, const char *)
�����@�@�@�@�Fconst char *message		�G���[���b�Z�[�W�Ƃ��ďo�͂��镶����
�@�@�@�@�@�@�@const char *body			HTML��BODY
�����@�@�@�@�Fmessage�Ŏw�肵���G���[���b�Z�[�W��BODY�̐F�y�[�W�֕\������B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void fatal_error(const char* message, const char* body)
{

    printPageHeader( "�v���I�G���[" );
    puts(body);
    puts("<BLOCKQUOTE><BR><BR>");
    puts(message);
    puts("</BLOCKQUOTE></BODY></HTML>");

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint getCountInt (const char *)
�����@�@�@�@�Fconst char *filename	�Ώۂ̃t�@�C��
�߂�l�@�@�@�Fint					�擾�����J�E���^
�����@�@�@�@�Ffilename����int�^�̐���ǂݍ��ݕԂ��B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int getCountInt(const char* file_path)
{

    int count;
    char buf[6];
    FILE *fp;

    if((fp = fopen(file_path,"r")) == NULL)
        fatal_error("�J�E���^�[�t�@�C�����J���܂���","<BODY>");

    fgets(buf,6,fp);
    count = atoi(buf);
    fclose(fp);

    return count;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint putCountInt( const char *, int )
�����@�@�@�@�Fconst char *filename	�Ώۂ̃t�@�C��
�@�@�@�@�@�@�@int count				int�^�̐�
�����@�@�@�@�Ffilename�Ŏw�肳�ꂽ�t�@�C����count���������ށB
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
�v���g�^�C�v�Fchar *check_url (char *)
�����@�@�@�@�Fchar *url		�`�F�b�N�Ώۂ�URL
�߂�l�@�@�@�Fchar *		�`�F�b�N��̕�����
�����@�@�@�@�Furl���������`���ł��邩���ׂ�
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
�v���g�^�C�v�Fvoid setCfValue(const char *, const char *, const char *)
�����@�@�@�@�Fconst char *cfFname		�R���t�B�M�����[�V�����t�@�C����
�@�@�@�@�@�@�@const char *keyName		�l���Z�b�g����L�[�̖��O
�@�@�@�@�@�@�@const char *keyValue		�Z�b�g����l
�����@�@�@�@�FcfFname�Ŏw�肳�ꂽ�R���t�B�M�����[�V�����t�@�C����ǂݍ��݁A
�@�@�@�@�@�@�@�Y������keyName�̍s��T���A�l��KeyValue�ɕύX����B
�@�@�@�@�@�@�@�������A�R���t�B�M�����[�V�����t�@�C���̏����͈ȉ��̒ʂ�
�@�@�@�@�@�@�@�܂��A��̃t�@�C�����ɊY������L�[�͈�������݂��Ȃ����̂�
�@�@�@�@�@�@�@����B
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
            fatal_error("�t�@�C���̃I�[�v���Ɏ��s���܂����B(setCfValue()-2)", "<BODY>");
            freeTwoDimArray(bufRead);	/*�̈���*/
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
        freeTwoDimArray(bufRead);	/*�̈���*/
        if(flag == 0)		/*�Y������L�[�����݂��Ȃ������ꍇ�͂��̃L�[���쐬����*/
            createCfKey(cfFname, KeyName, KeyValue);

    } else {
        createCfKey(cfFname, KeyName, KeyValue);
    }
    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid createCfKey(const char *, const char *, const char *)
�����@�@�@�@�Fconst char *cfFname		�R���t�B�M�����[�V�����t�@�C����
�@�@�@�@�@�@�@const char *keyName		��������L�[�̖��O
�@�@�@�@�@�@�@const char *keyValue		��������L�[�ɑ΂���l
�����@�@�@�@�FcfFname�Ŏw�肳�ꂽ�R���t�B�M�����[�V�����t�@�C���̍ŏI�s��
�@�@�@�@�@�@�@KeyName=KeyValue��ǉ�����B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void createCfKey(const char *cfFname, const char *KeyName, const char *KeyValue)
{

    FILE			*afp;

    if((afp = fopen(cfFname, "a")) == NULL) {
        fatal_error("�t�@�C���̃I�[�v���Ɏ��s���܂����B(setCfValue()-2)", "<BODY>");
        exit(-1);
    }

    fprintf(afp, "%s=%s\n", KeyName, KeyValue);
    fclose(afp);

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar *getFormdataValue(char *, char **, char **, int)
�����@�@�@�@�Fchar *name		�l���擾������name
�@�@�@�@�@�@�@char **dname		�t�H�[���f�[�^��name���i�[�����񎟔z��
�@�@�@�@�@�@�@char **dvalue		�t�H�[���f�[�^��value���i�[�����񎟔z��
�Ԃ�l�@�@�@�@char *			������name�ɑΉ�����value
�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@name�����݂��Ȃ������ꍇ��NULL
�����@�@�@�@�F������name�Ŏw�肵��value��Ԃ��܂��B
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
�v���g�^�C�v�Fchar *getValueAlloc(char *, char **, char **, int)
�����@�@�@�@�Fchar *name		�l���擾������name
�@�@�@�@�@�@�@char **dname		�t�H�[���f�[�^��name���i�[�����񎟔z��
�@�@�@�@�@�@�@char **dvalue		�t�H�[���f�[�^��value���i�[�����񎟔z��
�@�@�@�@�@�@�@int  count		�t�H�[���f�[�^name=value�̑g��
�Ԃ�l�@�@�@�@char *			������name�ɑΉ�����value
�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@name�����݂��Ȃ������ꍇ��\0�������ĕԂ��B
�����@�@�@�@�F������name�Ŏw�肵��value��Ԃ��܂��BgetValue�Ƃ̈Ⴂ�͗̈���m
�@�@�@�@�@�@�@�ۂ��ĕԂ����ǂ����ł���B
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
�v���g�^�C�v�Fchar *getCfValueStr(FILE *, const char *);
�����@�@�@�@�FFILE *rfp				�Y������t�@�C���ւ̃|�C���^
�Ԃ�l�@�@�@�@const char *KeyName	���o�������L�[�̖��O
�����@�@�@�@�Frfp�Ŏw�肳�ꂽ�t�@�C������KeyName�ɊY������Value��Ԃ��B
�@�@�@�@�@�@�@�������A�������Value�Ɍ���܂��B
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
�v���g�^�C�v�Fint getCfValueInt(FILE *, const char *);
�����@�@�@�@�FFILE *rfp				�Y������t�@�C���ւ̃|�C���^
�Ԃ�l�@�@�@�@const char *KeyName	���o�������L�[�̖��O
�����@�@�@�@�Frfp�Ŏw�肳�ꂽ�t�@�C������KeyName�ɊY������Value��Ԃ��B
�@�@�@�@�@�@�@�������Aint�^��Value�Ɍ���܂��B���݂��Ȃ��ꍇ��-1
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
�v���g�^�C�v�Fchar *myGetEnv(const char *)
�����@�@�@�@�Fconst char *envName	���ϐ���
�����@�@�@�@�FenvName�Ŏw�肵�����ϐ����擾������A�ʂ̗̈�ɒl���R�s�[���āA
�@�@�@�@�@�@�@���̗̈�ւ̃|�C���^��Ԃ��B���ϐ��̎擾�Ɏ��s�����ꍇ�́A
�@�@�@�@�@�@�@�uUnknown�v����������B�܂��A���̊֐��̎g�p��͊Y������̈��
�@�@�@�@�@�@�@�΂��āA�K��freeEnvironment()�����Ȃ���΂Ȃ�Ȃ��B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *myGetEnv(const char *envName)
{

    char *env;		/*���ϐ�*/
    char *buf;		/*BUFFER*/

    if((buf = getenv(envName)) == NULL) {	/*���ϐ��̎擾�Ɏ��s������*/
        env = (char *)malloc(sizeof(char) * 10);
        strcpy(env, "Unknown");
    } else {
        env = (char *)malloc(strlen(buf) + 1);
        strcpy(env, buf);
    }

    return env;

}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid freeEnvironment(char *)
�����@�@�@�@�F���ϐ��i�[�p�Ɋm�ۂ��ꂽ�̈���������
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void freeEnvironment(char *env)
{

    if(env)
        free(env);

    return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fvoid printPageHeader(const char *);
�����@�@�@�@�Fconst char *		�y�[�W�̃^�C�g��
�����@�@�@�@�FCGI�w�b�_��HTML�w�b�_���o�͂���
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
�v���g�^�C�v�Fvoid printUrl(char *)
�����@�@�@�@�Fchar *url		�o�͂���URL
�����@�@�@�@�F�Q�Ƃ���URL��\������B
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
        printf("\n\n<B>�� �Q�ƁF<A HREF=\"%s\" TARGET=\"_new\">%s</A></B></FONT>",url,url);

    return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar *rmHtmlTag(char *);
�����@�@�@�@�Fchar *str		���H���镶����
�����@�@�@�@�Fstr����HTML�^�O����������
�@�@�@�@�@�@�@��j<B>����������</B> �� ����������
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
