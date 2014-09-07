/*----------------------------------------------------------------------
 * libcgi.h
 * CGI汎用ユーティリティ用ヘッダファイル
 *
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
 *---------------------------------------------------------------------*/

#define BUFSIZE (1024)

int selectBrouser( void );
char tochar(char *);
void decode(char *);
int getForm(char ***, char ***);
char *getFormData( void );
void freedata(char **, char **);
char **getCookies( void );
char *getCookieRecord( char *, char ** );
int dataSeparater( char ***, char ***, char *);
int getCookieData(char ***, char ***, char *);
void fatal_error(const char*, const char*);
int getCountInt(const char*);
int putCountInt(const char*, int);
int check_url(char *);
char *myGetEnv(const char *);
void freeEnvironment(char *);
void setCfValue(const char *, const char *, const char *);
void createCfKey(const char *, const char *, const char *);
int getCfValueInt( char **, const char * );
char *getCfValueStr( char **, const char *, char *, size_t );
char *getValue(char *, char **, char **);
char *getValueAlloc(char *, char **, char **);
void printPageHeader(const char *);
void printUrl(char *);
char *rmHtmlTag(char *);