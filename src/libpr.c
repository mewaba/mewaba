/*---------------------------------------------------------------------------
 * libpr.c Ver1.12
 * 一般実用ライブラリ
 *
 * Last update : 1999/4/13
 *
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
----------------------------------------------------------------------------*/

/*--- Include ---*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>		/*#ifdef LOCK_SH*/
#include "libpr.h"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int delline (char *, unsigned int)
引数　　　　：char *fName	対象となるファイル名
　　　　　　　unsigned int 		削除する行（1からカウントした数）
説明　　　　：指定されたファイルの指定行を削除する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int delLine(char *fName, unsigned int lNumber){

	FILE			*fp;
	unsigned int	i;
	char			**buf2Dim;

	buf2Dim = readFile( fName );
	if( buf2Dim == NULL )
		return 0;

	if((fp = fopen(fName, "w")) == NULL)
		return 0;
	for (i = 0; *(buf2Dim + i) ; i++){
		if((i + 1) != lNumber)
			fputs(*(buf2Dim + i),fp);
	}
	fclose(fp);

	freeTwoDimArray(buf2Dim);

	return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int addLine (char *, char *)
引数　　　　：char *filename		対象となるファイル名
　　　　　　　char *host_name 		ホスト名
説明　　　　：指定されたファイルに項目を追加する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int addLine(char *fName, char *addString){

	FILE	*fp;

	if((fp = fopen(fName, "a")) == NULL)
		return 0;

	fprintf(fp,"%s\n", addString);

	fclose(fp);

	return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int addLineTop(const char *, const char *);
引数　　　　：const char *fName		書き込むファイル名
　　　　　　　const char *str		書き込む内容
返り値　　　　int				1:正常終了
　　　　　　　					0:異常終了
説明　　　　：fpで指定されるファイルの先頭に１行（str）書き込む。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int addLineTop(const char *fName, const char *str){

	unsigned int	i;
	char			**bufRead;
	FILE			*wfp;

	bufRead = readFile(fName);

	if((wfp = fopen(fName, "w")) == NULL)
		return 0;

	fprintf(wfp, "%s\n" ,str);
	for(i = 0; *(bufRead + i); i++)
		fputs(*(bufRead + i), wfp);

	fclose(wfp);

	freeTwoDimArray(bufRead);

	return 1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void freeTwoDimArray(char **, unsigned int)
引数　　　　：char **Array			解放する二次配列へのポインタ
　　　　　　　unsigned int lines	解放する二次配列の配列数
説明　　　　：動的に確保された二次配列Arrayを全て解放します。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void freeTwoDimArray(char **Array){

	unsigned int	i;

	if( Array != NULL ){
		for( i = 0; *(Array + i); i++ )
				free(*(Array + i));

		if(Array)
			free(Array);
	}

	return;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *fgetLine(FILE *);
引数　　　　：FILE *rfp		読み込むファイルへのポインタ
返り値　　　　char *		読み込んだ１行
　　　　　　　　　　　　　　※ファイルの終わりの場合はNULL
説明　　　　：rfpで指定されたファイルから１行読み込み、その文字列を格納した
　　　　　　　領域へのポインタを返す。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *fgetLine(FILE *rfp){

	char	buf[BUFSIZE];
	char	*tmp = NULL;

	while(fgets(buf, BUFSIZE, rfp) != NULL){
		if(!tmp){
			tmp = (char *)malloc(strlen(buf) + 1);
			if(tmp == NULL){
				perror("Memory allocation error.");
				return NULL;
			}
			strcpy(tmp, buf);
		}else{
			tmp = (char *)realloc((char *)tmp, strlen(tmp) + strlen(buf) + 1);
			if(tmp == NULL){
				perror("Memory reallocation error.");
				return NULL;
			}
			strcat(tmp, buf);
		}
		if(isNewLine(buf))
			return tmp;
		else if(strlen(buf) < BUFSIZE)
			return tmp;

	}
	return NULL;

}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char **readFile(const char *, unsigned int)
引数　　　　：const char *fName		読み込むファイル名
　　　　　　　unsigned int lines	読み込むファイルの行数
戻り値　　　：char **				該当するファイルの内容を格納した二次配列
説明　　　　：fNameで指定されたファイルを読み込み、内容を格納した二次配列への
　　　　　　　ポインタを返す。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char **readFile(const char *fName){

	char			*buf;
	char			**bufRead = NULL;
	FILE			*rfp;
	unsigned int	i = 0;

	if((rfp = fopen(fName, "r")) == NULL)
		return NULL;

	while((buf = fgetLine(rfp)) != NULL){
		bufRead = (char **)realloc(bufRead, sizeof(char *) * (i + 1));
		if(bufRead == NULL){
			perror("Memory reallocation error.");
			return NULL;
		}
		*(bufRead + i) = buf;
		i++;
	}
	bufRead = (char **)realloc(bufRead, sizeof(char *) * (i + 1));
	if(bufRead == NULL){
		perror("Memory reallocation error.");
		return NULL;
	}
	*(bufRead + i) = NULL;
	fclose( rfp );

	return bufRead;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：void removeNewline (char *)
引数　　　　：char *obj		対象となる文字列
説明　　　　：obj中にCR、またはLFが存在した場合出てきた時点で文字列を切る
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void removeNewline(char *obj){

	char	*p = obj;	/*objの位置保存*/

	while(*obj){
		if(*obj != 0x0a && *obj != 0x0d)
			obj++;		
		else
			*obj = '\0';
	}
	obj = p;	/*位置復元*/

	return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int isNewLine(char *);
引数　　　　：char *str		チェックする文字列
返り値　　　　int			0：改行が存在しない
　　　　　　　　　　　　　　1：改行が存在する
説明　　　　：strに改行が存在するかどうか調べる
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int isNewLine(char *str){

	char	*s = str;

	while(*str){
		if(*str == 0x0a || *str == 0x0d){
			str = s;
			return 1;
		}
		str++;
	}
	str = s;
	return 0;

}

char *emptyAlloc( void ){
	
	char	*empty;

	empty = (char *)malloc( sizeof(char) * 1 );
	if( empty == NULL )
		return NULL;
	*empty = '\0';
	
	return empty;

}

#ifndef LOCK_SH	/*---flockがないOSでの処理---*/

int f_lock( int fd ){

	while( lockf(fd, F_TLOCK, 0) == -1 ){
		if( errno != EAGAIN )
			return -1;
	}
	return 0;

}

int f_unlock( int fd ){

	while( lockf(fd, F_ULOCK, 0) == -1 ){
		if( errno == EBADF || errno == ECOMM )
			return -1;
	}

	return 0;

}

#else	/*---flockがあるOSでの処理---*/

int f_lock ( int fd ){

	if(flock(fd, LOCK_EX) == -1)
		return -1;

	return 0;

}


int f_unlock ( int fd ){

	if(flock(fd, LOCK_UN) == -1)
		return -1;

	return 0;

}

#endif
/*End of file*/