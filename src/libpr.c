/*---------------------------------------------------------------------------
 * libpr.c Ver1.12
 * ��ʎ��p���C�u����
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
�v���g�^�C�v�Fint delline (char *, unsigned int)
�����@�@�@�@�Fchar *fName	�ΏۂƂȂ�t�@�C����
�@�@�@�@�@�@�@unsigned int 		�폜����s�i1����J�E���g�������j
�����@�@�@�@�F�w�肳�ꂽ�t�@�C���̎w��s���폜����
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
�v���g�^�C�v�Fint addLine (char *, char *)
�����@�@�@�@�Fchar *filename		�ΏۂƂȂ�t�@�C����
�@�@�@�@�@�@�@char *host_name 		�z�X�g��
�����@�@�@�@�F�w�肳�ꂽ�t�@�C���ɍ��ڂ�ǉ�����
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
�v���g�^�C�v�Fint addLineTop(const char *, const char *);
�����@�@�@�@�Fconst char *fName		�������ރt�@�C����
�@�@�@�@�@�@�@const char *str		�������ޓ��e
�Ԃ�l�@�@�@�@int				1:����I��
�@�@�@�@�@�@�@					0:�ُ�I��
�����@�@�@�@�Ffp�Ŏw�肳���t�@�C���̐擪�ɂP�s�istr�j�������ށB
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
�v���g�^�C�v�Fvoid freeTwoDimArray(char **, unsigned int)
�����@�@�@�@�Fchar **Array			�������񎟔z��ւ̃|�C���^
�@�@�@�@�@�@�@unsigned int lines	�������񎟔z��̔z��
�����@�@�@�@�F���I�Ɋm�ۂ��ꂽ�񎟔z��Array��S�ĉ�����܂��B
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
�v���g�^�C�v�Fchar *fgetLine(FILE *);
�����@�@�@�@�FFILE *rfp		�ǂݍ��ރt�@�C���ւ̃|�C���^
�Ԃ�l�@�@�@�@char *		�ǂݍ��񂾂P�s
�@�@�@�@�@�@�@�@�@�@�@�@�@�@���t�@�C���̏I���̏ꍇ��NULL
�����@�@�@�@�Frfp�Ŏw�肳�ꂽ�t�@�C������P�s�ǂݍ��݁A���̕�������i�[����
�@�@�@�@�@�@�@�̈�ւ̃|�C���^��Ԃ��B
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
�v���g�^�C�v�Fchar **readFile(const char *, unsigned int)
�����@�@�@�@�Fconst char *fName		�ǂݍ��ރt�@�C����
�@�@�@�@�@�@�@unsigned int lines	�ǂݍ��ރt�@�C���̍s��
�߂�l�@�@�@�Fchar **				�Y������t�@�C���̓��e���i�[�����񎟔z��
�����@�@�@�@�FfName�Ŏw�肳�ꂽ�t�@�C����ǂݍ��݁A���e���i�[�����񎟔z��ւ�
�@�@�@�@�@�@�@�|�C���^��Ԃ��B
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
�v���g�^�C�v�Fvoid removeNewline (char *)
�����@�@�@�@�Fchar *obj		�ΏۂƂȂ镶����
�����@�@�@�@�Fobj����CR�A�܂���LF�����݂����ꍇ�o�Ă������_�ŕ������؂�
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
void removeNewline(char *obj){

	char	*p = obj;	/*obj�̈ʒu�ۑ�*/

	while(*obj){
		if(*obj != 0x0a && *obj != 0x0d)
			obj++;		
		else
			*obj = '\0';
	}
	obj = p;	/*�ʒu����*/

	return;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint isNewLine(char *);
�����@�@�@�@�Fchar *str		�`�F�b�N���镶����
�Ԃ�l�@�@�@�@int			0�F���s�����݂��Ȃ�
�@�@�@�@�@�@�@�@�@�@�@�@�@�@1�F���s�����݂���
�����@�@�@�@�Fstr�ɉ��s�����݂��邩�ǂ������ׂ�
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

#ifndef LOCK_SH	/*---flock���Ȃ�OS�ł̏���---*/

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

#else	/*---flock������OS�ł̏���---*/

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