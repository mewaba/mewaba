/*---------------------------------------------------------------------------
 * libstr.c Ver1.2
 * �����񑀍색�C�u����
 *
 * Last update : 1999/4/17
 *
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
 * compile: gcc -c libstr.c
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fint strspl (char *, char *, char);
�����@�@�@�@�Fchar *s1		���������̒��O�܂ł��i�[���镶����
�@�@�@�@�@�@�@char *s2		���H�Ώە�����
�@�@�@�@�@�@�@char search	��������
�߂�l�@�@�@�Fint			���H�Ώە�����̏I�[�F0
�����@�@�@�@�Fs2����search��������܂�s1�ɑ�����As2�̎c������ɃV�t�g����
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int strspl(char *s1, char *s2, char search)
{

    int flag = 1;
    char *p = s2, *q = s2;		/*s2�̈ʒu�ۑ�*/

    if(s1 == NULL || s2 == NULL)
        return 0;

    while(*p != search && *p)	/*search�ɓ����钼�O�܂ł̒l��*out�ɑ�� */
        *s1++ = *p++;

    if(!*p)						/*�Y�����镶������*/
        flag = 0;
    *s1 = '\0';				/*s1�̍Ō��\0���*/
    p++;					/*��؂蕶���̕������X�L�b�v*/
    while(*p)				/*�c��̕�������A�擪�ɃV�t�g������ */
        *q++ = *p++;
    *q = '\0';
    return flag;
}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar *strsub (char *, const char *, int, int)
�����@�@�@�@�Fchar *s1		�o�͕�����
�@�@�@�@�@�@�@char *s2		�����Ώە�����
�@�@�@�@�@�@�@int start		�J�n�ʒu
�@�@�@�@�@�@�@int end		�I���ʒu
�߂�l�@�@�@�Fchar *		s2��start����end�܂ł̕�����
�����@�@�@�@�Fs2��start����end�܂ł�s1�֔����o���B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *strsub(char *s1, const char *s2, int start, int end)
{

    char	*p = s1;			/*s1�̈ʒu�ۑ�*/

    if(s1 == NULL || s2 == NULL)
        return NULL;

    s2 += start;				/*s2�̃|�C���^���w�萔�i�߂�*/
    while(start < end-- && *s2)	/*�w�蕶������s1�փR�s�[����*/
        *p++ = *s2++;

    *p = '\0';

    return s1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar *replacestring (char *, int, int)
�����@�@�@�@�Fchar *in		�u���Ώە�����
�@�@�@�@�@�@�@char search	����������
�@�@�@�@�@�@�@char *replace	�u��������
�߂�l�@�@�@�Fchar *		�u���I����̕�����ւ̃|�C���^
�����@�@�@�@�Fin����search���������Areplace�ɒu��������B
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *replacestring(char *strIn, char *strSearch, char *strReplace)
{

    unsigned int		lenInput = 0;
    unsigned int		lenSearch = 0;
    unsigned int		lenBuffer = 0;
    unsigned int		lenReplace = 0;
    unsigned int		sizeDest = 0;
    unsigned int		len = 0;
    char*				s;
    char*				dest = NULL;

    if(strIn == NULL || strSearch == NULL || strReplace == NULL)
        return strIn;
    else if(strlen(strIn) == 0)
        return NULL;

    lenSearch = strlen(strSearch);
    lenReplace = strlen(strReplace);

    sizeDest = (lenReplace + (lenReplace % lenSearch)) / lenSearch;
    if((dest = (char *)malloc(strlen(strIn) * (sizeDest + 1))) == NULL)
        return NULL;
    memset( (char *)dest, 0, sizeof( dest ) );

    if(lenSearch <= 0) {		/*���������񒷂�0�̏ꍇ�͕���������̂܂ܕԂ�*/
        strcpy(dest, strIn);
        return dest;
    }
    while(1) {
        lenInput = strlen(strIn);		/*���͕�����*/
        if((s = strstr(strIn, strSearch)) == NULL) {
            strcat(dest, strIn);
            break;
        }
        lenBuffer = strlen(s);
        len = lenInput - lenBuffer;
        strncat(dest, strIn, len);
        strIn += len;
        strcat(dest, strReplace);
        s += lenSearch;
        strIn = s;
    }
    return dest;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
�v���g�^�C�v�Fchar *trLower(char *)
�����@�@�@�@�Fchar *str		�ϊ��Ώۂ̕�����
�߂�l�@�@�@�Fchar*			�ϊ���̕�����
�����@�@�@�@�Fstr��S�ď������ɕϊ�����
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *trLower(char *str)
{

    char *p = str;

    while(*str++) {
        if('A' <= *str && *str <= 'Z')
            *str += ' ';
    }
    str = p;

    return str;
}

/*-----------------------------------------------------------------------------
ProtoType  : char *strhead( char *s1, const char *s2, int c );
Arguments  : char s1 ... �i�[��ϐ��i�Œ�s2���i�[�ł���T�C�Y���m�ۂ���Ă���K�v������j
             const char s2 ... ���ʌ��ϐ�
		     int c  ... ���������ichar�ŃL���X�g����j
Returns    : char * ... s1�̐擪�A�h���X
Description: s2����c��T���A������܂�s1�֑������
-----------------------------------------------------------------------------*/
char *strhead( char *s1, char *s2, int c )
{

    char *p = s1;	/*�ʒu�ۑ�*/

    while( *s2 != (char)c && *s2)
        *p++ = *s2++;
    *p = '\0';

    return s1;

}
