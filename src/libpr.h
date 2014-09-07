/*----------------------------------------------------------------------
 * libpr.h
 * 一般実用ライブラリヘッダファイル
 *
 * Copyright(C) 1997-98  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
 *---------------------------------------------------------------------*/
#define	BUFSIZE	(1024)

int delLine (char *, unsigned int);
int addLine (char *, char *);
void freeTwoDimArray(char **);
char *fgetLine(FILE *);
char **readFile(const char *);
void removeNewline (char *);
int isNewLine(char *);
int addLineTop(const char *, const char *);
char *emptyAlloc( void );
int f_lock( int );
int f_unlock( int );
